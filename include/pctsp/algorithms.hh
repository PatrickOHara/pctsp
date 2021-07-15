#ifndef __PCTSP_PCTSP__
#define __PCTSP_PCTSP__
/** Exact algorithms for PCTSP */

#include <iostream>
#include "scip/message_default.h"

#include "constraint.hh"
#include "heuristic.hh"
#include "logger.hh"
#include "preprocessing.hh"
#include "solution.hh"
#include "subtour_elimination.hh"
using namespace boost;
using namespace scip;
using namespace std;

template <typename Edge>
const char*
getVariableNameFromEdge(std::map<Edge, SCIP_VAR*>& edge_variable_map,
    Edge edge) {
    SCIP_VAR* variable = edge_variable_map[edge];
    const char* variable_name = SCIPvarGetName(variable);
    return variable_name;
}

/** Add edge variables to the PCTSP SCIP model */
template <typename Graph, typename CostMap, typename VariableMap>
SCIP_RETCODE PCTSPaddEdgeVariables(SCIP* mip, Graph& graph, CostMap& cost_map,
    VariableMap& variable_map) {
    for (auto edge : make_iterator_range(edges(graph))) {
        SCIP_VAR* edge_variable;
        int cost_of_edge = cost_map[edge];
        SCIP_CALL(SCIPcreateVar(mip, &edge_variable, NULL, 0.0, 1.0,
            cost_of_edge, SCIP_VARTYPE_BINARY, TRUE, FALSE,
            NULL, NULL, NULL, NULL, NULL));

        SCIP_CALL(SCIPaddVar(mip, edge_variable));
        variable_map[edge] = edge_variable;
        // release variable after adding it to model
        // SCIP_CALL(SCIPreleaseVar(mip, &edge_variable));
    }
    return SCIP_OKAY;
}

SCIP_RETCODE addHeuristicVarsToSolver(
    SCIP* scip,
    SCIP_HEUR* heur,
    std::vector<SCIP_VAR*>& vars
);

template <typename Graph, typename EdgeVariableMap, typename EdgeIt>
SCIP_RETCODE addHeuristicEdgesToSolver(
    SCIP* scip,
    Graph& graph,
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

    return addHeuristicVarsToSolver(scip, heur, vars_of_edges);
}

template <typename Graph, typename EdgeVariableMap, typename VertexIt>
SCIP_RETCODE addHeuristicTourToSolver(
    SCIP* scip,
    Graph& graph,
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
template <typename Graph, typename Vertex, typename CostMap, typename WeightMap>
SCIP_RETCODE PCTSPmodelWithoutSECs(
    SCIP* mip, Graph& graph, CostMap& cost_map, WeightMap& weight_map,
    int quota, Vertex root_vertex,
    std::map<typename Graph::edge_descriptor, SCIP_VAR*>& variable_map) {
    // from the graph, create the variables on edges and nodes
    SCIP_CALL(PCTSPaddEdgeVariables(mip, graph, cost_map, variable_map));
    int nvars = SCIPgetNVars(mip);

    // add objective function
    SCIP_CALL(SCIPsetObjsense(mip, SCIP_OBJSENSE_MINIMIZE));
    // add the prize constraint to the solver
    SCIP_CALL(
        PCTSPaddPrizeConstraint(mip, variable_map, weight_map, quota, nvars));

    // add constraint that the root vertex must be visited
    auto root_vd = vertex(root_vertex, graph); // get vertex descriptor
    auto root_self_loop = edge(root_vd, root_vd, graph);
    if (root_self_loop.second == false) {
        throw EdgeNotFoundException(std::to_string(root_vd),
            std::to_string(root_vd));
    }
    SCIP_CALL(
        PCTSPaddRootVertexConstraint(mip, variable_map, root_self_loop.first));

    // add constraint to ensure a vertex is adjacent to exactly two edges in
    // the tour
    SCIP_CALL(PCTSPaddDegreeTwoConstraint(mip, graph, variable_map));

    return SCIP_OKAY;
}

/** Solve the Prize Collecting TSP problem using a branch and cut algorithm
 */
template <typename Graph, typename CostMap, typename PrizeMap>
SCIP_RETCODE PCTSPbranchAndCut(
    Graph& graph,
    std::vector<typename boost::graph_traits<Graph>::edge_descriptor>& solution_edges,
    CostMap& cost_map,
    PrizeMap& prize_map,
    int quota,
    typename boost::graph_traits<Graph>::vertex_descriptor root_vertex,
    const char* log_filepath = NULL,
    bool print_scip = true,
    bool sec_disjoint_tour = true,
    int sec_disjoint_tour_freq = 1,
    bool sec_maxflow_mincut = true,
    int sec_maxflow_mincut_freq = 1,
    float time_limit = 14400    //  seconds (default 4 hours)
) {
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

    // initialise empty model
    SCIP* mip = NULL;
    SCIP_CALL(SCIPcreate(&mip));
    SCIP_CALL(SCIPincludeDefaultPlugins(mip));

    // datastructures needed for the MIP solver
    std::map<Edge, SCIP_VAR*> edge_variable_map;
    std::map<Edge, int> weight_map;
    ProbDataPCTSP* probdata = new ProbDataPCTSP(&graph, &root_vertex, &edge_variable_map, &quota);
    SCIPcreateObjProb(
        mip,
        "test-pctsp-with-secs",
        probdata,
        true
    );

    // add custom message handler
    SCIP_MESSAGEHDLR* handler;
    SCIP_CALL(SCIPcreateMessagehdlrDefault(&handler, false, log_filepath, print_scip));
    SCIP_CALL(SCIPsetMessagehdlr(mip, handler));

    // add custom cutting plane handlers
    SCIPincludeObjConshdlr(mip, new PCTSPconshdlrSubtour(mip, sec_disjoint_tour, sec_disjoint_tour_freq, sec_maxflow_mincut, sec_maxflow_mincut_freq), TRUE);

    BOOST_LOG_TRIVIAL(info) << "Created SCIP program. Adding constraints and variables.";

    // move prizes of vertices onto the weight of an edge
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // add variables and constraints to SCIP model
    SCIP_CALL(PCTSPmodelWithoutSECs(mip, graph, cost_map, weight_map, quota,
        root_vertex, edge_variable_map));
    int nvars = SCIPgetNVars(mip);

    // turn off presolving
    SCIPsetIntParam(mip, "presolving/maxrounds", 0);

    // time limit
    SCIPsetRealParam(mip, "limits/time", time_limit);

    // add the subtour elimination constraints as cutting planes
    SCIP_CONS* cons;
    std::string cons_name("subtour-constraint");
    PCTSPcreateBasicConsSubtour(mip, &cons, cons_name, graph, root_vertex);
    SCIPaddCons(mip, cons);
    SCIPreleaseCons(mip, &cons);

    // TODO add the cost cover inequalities as cutting planes

    // add selected heuristics to reduce the upper bound on the optimal
    if (solution_edges.size() > 0) {
        auto first = solution_edges.begin();
        auto last = solution_edges.end();
        BOOST_LOG_TRIVIAL(info) << "Adding starting solution to solver.";
        addHeuristicEdgesToSolver(mip, graph, NULL, edge_variable_map, first, last);
    }

    // TODO adjust parameters for the branching strategy

    BOOST_LOG_TRIVIAL(info) << "Added contraints and variables. Solving model.";
    // Solve the model
    SCIP_CALL(SCIPsolve(mip));
    BOOST_LOG_TRIVIAL(info) << "Model solved. Getting edge list of best solution.";
    // Get the solution
    SCIP_SOL* sol = SCIPgetBestSol(mip);
    solution_edges = getSolutionEdges(mip, graph, sol, edge_variable_map);
    if (print_scip) {
        BOOST_LOG_TRIVIAL(info) << "Saving SCIP logs to: " << log_filepath;
        FILE* log_file = fopen(log_filepath, "w");
        SCIP_CALL(SCIPprintOrigProblem(mip, log_file, NULL, true));
        SCIP_CALL(SCIPprintBestSol(mip, log_file, true));
        SCIP_CALL(SCIPprintStatistics(mip, log_file));
    }
    BOOST_LOG_TRIVIAL(debug) << "Releasing constraint handler.";
    SCIP_CALL(SCIPmessagehdlrRelease(&handler));
    BOOST_LOG_TRIVIAL(debug) << "Releasing SCIP model.";
    SCIP_CALL(SCIPfree(&mip));
    BOOST_LOG_TRIVIAL(debug) << "Done releasing model. Returning status SCIP_OKAY.";
    return SCIP_OKAY;
}
#endif