#ifndef __PCTSP_PCTSP__
#define __PCTSP_PCTSP__
/** Exact algorithms for PCTSP */

#include <iostream>
#include "scip/message_default.h"

#include "constraint.hh"
#include "cost_cover.hh"
#include "event_handlers.hh"
#include "heuristic.hh"
#include "logger.hh"
#include "preprocessing.hh"
#include "solution.hh"
#include "stats.hh"
#include "subtour_elimination.hh"
using namespace boost;
using namespace scip;
using namespace std;

template <typename TEdge>
const char*
getVariableNameFromEdge(std::map<TEdge, SCIP_VAR*>& edge_variable_map,
    TEdge edge) {
    SCIP_VAR* variable = edge_variable_map[edge];
    const char* variable_name = SCIPvarGetName(variable);
    return variable_name;
}

/** Add edge variables to the PCTSP SCIP model */
template <typename TGraph, typename TCostMap, typename VariableMap>
SCIP_RETCODE PCTSPaddEdgeVariables(SCIP* scip, TGraph& graph, TCostMap& cost_map,
    VariableMap& variable_map) {
    for (auto edge : make_iterator_range(edges(graph))) {
        SCIP_VAR* edge_variable;
        int cost_of_edge = cost_map[edge];
        SCIP_CALL(SCIPcreateVar(scip, &edge_variable, NULL, 0.0, 1.0,
            cost_of_edge, SCIP_VARTYPE_BINARY, TRUE, FALSE,
            NULL, NULL, NULL, NULL, NULL));

        SCIP_CALL(SCIPaddVar(scip, edge_variable));
        variable_map[edge] = edge_variable;
        // release variable after adding it to model
        // SCIP_CALL(SCIPreleaseVar(scip, &edge_variable));
    }
    return SCIP_OKAY;
}

SCIP_RETCODE addHeuristicVarsToSolver(
    SCIP* scip,
    SCIP_HEUR* heur,
    std::vector<SCIP_VAR*> vars
);

template <typename TGraph, typename EdgeVariableMap, typename EdgeIt>
SCIP_RETCODE addHeuristicEdgesToSolver(
    SCIP* scip,
    TGraph& graph,
    SCIP_HEUR* heur,
    EdgeVariableMap& edge_variable_map,
    EdgeIt& first,
    EdgeIt& last
) {
    auto n_edges = std::distance(first, last);
    auto vertex_vector = getVerticesOfEdges(graph, first, last);
    std::advance(first, -n_edges);  // move iterator back to beginning
    auto vertex_first = vertex_vector.begin();
    auto vertex_last = vertex_vector.end();
    auto self_loops = getSelfLoops(graph, vertex_first, vertex_last);
    auto loops_first = self_loops.begin();
    auto loops_last = self_loops.end();
    auto vars_of_self_loops = getEdgeVariables(scip, graph, edge_variable_map, loops_first, loops_last);
    auto vars_of_edges = getEdgeVariables(scip, graph, edge_variable_map, first, last);
    vars_of_edges.insert(vars_of_edges.end(), vars_of_self_loops.begin(), vars_of_self_loops.end());
    SCIP_CALL(addHeuristicVarsToSolver(scip, heur, vars_of_edges));
    return SCIP_OKAY;
}

template <typename TGraph, typename EdgeVariableMap, typename VertexIt>
SCIP_RETCODE addHeuristicTourToSolver(
    SCIP* scip,
    TGraph& graph,
    SCIP_HEUR* heur,
    EdgeVariableMap& edge_variable_map,
    VertexIt& first,
    VertexIt& last
) {
    auto self_loops = getSelfLoops(graph, first, last);
    auto edges_of_walk = getEdgesInWalk(graph, first, last);

    auto vars_of_self_loops = getEdgeVariables(scip, graph, edge_variable_map, self_loops.begin(), self_loops.end());
    auto vars_of_edges = getEdgeVariables(scip, graph, edge_variable_map, edges_of_walk.begin(), edges_of_walk.end());
    vars_of_edges.insert(vars_of_edges.end(), vars_of_self_loops.begin(), vars_of_self_loops.end());

    return addHeuristicVarsToSolver(scip, heur, vars_of_edges);
}

/** Get a SCIP model of the prize-collecting TSP without any subtour
 * elimiation constraints (SECs).
 *
 * This function sets the objective function, adds the variables,
 * and adds the constraints.
 */
template <typename TGraph, typename TCostMap, typename WeightMap>
SCIP_RETCODE PCTSPmodelWithoutSECs(
    SCIP* scip,
    TGraph& graph,
    TCostMap& cost_map,
    WeightMap& weight_map,
    int& quota,
    typename TGraph::vertex_descriptor& root_vertex,
    std::map<typename TGraph::edge_descriptor, SCIP_VAR*>& variable_map) {
    // from the graph, create the variables on edges and nodes
    SCIP_CALL(PCTSPaddEdgeVariables(scip, graph, cost_map, variable_map));
    int nvars = SCIPgetNVars(scip);

    // add objective function
    SCIP_CALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE));
    // add the prize constraint to the solver
    SCIP_CALL(
        PCTSPaddPrizeConstraint(scip, variable_map, weight_map, quota, nvars));

    // add constraint that the root vertex must be visited
    auto root_vd = vertex(root_vertex, graph); // get vertex descriptor
    auto root_self_loop = edge(root_vd, root_vd, graph);
    if (root_self_loop.second == false) {
        throw EdgeNotFoundException(std::to_string(root_vd),
            std::to_string(root_vd));
    }
    SCIP_CALL(
        PCTSPaddRootVertexConstraint(scip, variable_map, root_self_loop.first));

    // add constraint to ensure a vertex is adjacent to exactly two edges in
    // the tour
    SCIP_CALL(PCTSPaddDegreeTwoConstraint(scip, graph, variable_map));

    return SCIP_OKAY;
}

/** Solve the Prize Collecting TSP problem using a branch and cut algorithm
 */
template <typename TGraph, typename TCostMap, typename TPrizeMap>
SCIP_RETCODE PCTSPbranchAndCut(
    TGraph& graph,
    std::vector<typename boost::graph_traits<TGraph>::edge_descriptor>& solution_edges,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    int quota,
    typename boost::graph_traits<TGraph>::vertex_descriptor root_vertex,
    std::string bounds_csv_filepath = "bounds.csv",
    bool cost_cover_disjoint_paths = false,
    bool cost_cover_shortest_path = true,
    bool cost_cover_steiner_tree = false,
    std::vector<int> disjoint_paths_distances = std::vector<int>(),
    std::string log_filepath = "scip_logs.txt",
    std::string metrics_csv_filepath = "metrics.csv",
    std::string name = "pctsp",
    bool sec_disjoint_tour = true,
    int sec_disjoint_tour_freq = 1,
    bool sec_maxflow_mincut = true,
    int sec_maxflow_mincut_freq = 1,
    float time_limit = 14400    //  seconds (default 4 hours)
) {
    typedef typename boost::graph_traits<TGraph>::edge_descriptor Edge;
    typedef typename boost::graph_traits<TGraph>::vertex_descriptor Vertex;

    // initialise empty model
    SCIP* scip = NULL;
    SCIP_CALL(SCIPcreate(&scip));
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));

    // datastructures needed for the MIP solver
    std::vector<NodeStats> node_stats;  // save node statistics
    std::map<Edge, SCIP_VAR*> edge_variable_map;
    std::map<Edge, int> weight_map;
    ProbDataPCTSP* probdata = new ProbDataPCTSP(&graph, &root_vertex, &edge_variable_map, &quota, &node_stats);
    SCIPcreateObjProb(
        scip,
        name.c_str(),
        probdata,
        true
    );

    // add custom message handler
    SCIP_MESSAGEHDLR* handler;
    SCIP_CALL(SCIPcreateMessagehdlrDefault(&handler, false, log_filepath.c_str(), true));
    SCIP_CALL(SCIPsetMessagehdlr(scip, handler));

    // add custom cutting plane handlers
    SCIP_CALL(SCIPincludeObjConshdlr(scip, new PCTSPconshdlrSubtour(scip, sec_disjoint_tour, sec_disjoint_tour_freq, sec_maxflow_mincut, sec_maxflow_mincut_freq), TRUE));

    // add event handlers
    SCIP_CALL( SCIPincludeObjEventhdlr(scip, new NodeEventhdlr(scip), TRUE) );
    BoundsEventHandler* bounds_handler = new BoundsEventHandler(scip);
    SCIP_CALL( SCIPincludeObjEventhdlr(scip, bounds_handler, TRUE));

    // add the cost cover inequalities when a new solution is found
    if (cost_cover_disjoint_paths) {
        SCIP_CALL(includeDisjointPathsCostCover(scip, disjoint_paths_distances));
    }
    if (cost_cover_shortest_path) {
        SCIP_CALL(includeShortestPathCostCover(scip, graph, cost_map, root_vertex));
    }


    BOOST_LOG_TRIVIAL(info) << "Created SCIP program. Adding constraints and variables.";

    // move prizes of vertices onto the weight of an edge
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // add variables and constraints to SCIP model
    SCIP_CALL(PCTSPmodelWithoutSECs(scip, graph, cost_map, weight_map, quota,
        root_vertex, edge_variable_map));
    int nvars = SCIPgetNVars(scip);

    // turn off presolving
    SCIPsetIntParam(scip, "presolving/maxrounds", 0);

    // time limit
    SCIPsetRealParam(scip, "limits/time", time_limit);

    // add the subtour elimination constraints as cutting planes
    SCIP_CONS* cons;
    std::string cons_name("subtour-constraint");
    PCTSPcreateBasicConsSubtour(scip, &cons, cons_name, graph, root_vertex);
    SCIPaddCons(scip, cons);
    SCIPreleaseCons(scip, &cons);

    // add selected heuristics to reduce the upper bound on the optimal
    if (solution_edges.size() > 0) {
        auto first = solution_edges.begin();
        auto last = solution_edges.end();
        BOOST_LOG_TRIVIAL(info) << "Adding starting solution to solver.";
        SCIP_HEUR* heur = NULL;
        addHeuristicEdgesToSolver(scip, graph, heur, edge_variable_map, first, last);
    }

    // TODO adjust parameters for the branching strategy

    BOOST_LOG_TRIVIAL(info) << "Added contraints and variables. Solving model.";
    // Solve the model
    SCIP_CALL(SCIPsolve(scip));
    BOOST_LOG_TRIVIAL(info) << "Model solved. Getting edge list of best solution.";

    // Get the solution
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    solution_edges = getSolutionEdges(scip, graph, sol, edge_variable_map);
    BOOST_LOG_TRIVIAL(info) << "Saving SCIP logs to: " << log_filepath;
    FILE* log_file = fopen(log_filepath.c_str(), "w");
    SCIP_CALL(SCIPprintOrigProblem(scip, log_file, NULL, true));
    SCIP_CALL(SCIPprintBestSol(scip, log_file, true));
    SCIP_CALL(SCIPprintStatistics(scip, log_file));

    // Write the bounds to file
    std::vector<Bounds> bounds_vector = bounds_handler->getBoundsVector();
    writeBoundsToCSV(bounds_vector, bounds_csv_filepath);

    // Get the metrics and statistics of the solver
    writeNodeStatsToCSV(node_stats, metrics_csv_filepath);

    BOOST_LOG_TRIVIAL(debug) << "Releasing constraint handler.";
    SCIP_CALL(SCIPmessagehdlrRelease(&handler));
    BOOST_LOG_TRIVIAL(debug) << "Releasing SCIP model.";
    SCIP_CALL(SCIPfree(&scip));
    BOOST_LOG_TRIVIAL(debug) << "Done releasing model. Returning status SCIP_OKAY.";
    return SCIP_OKAY;
}
#endif