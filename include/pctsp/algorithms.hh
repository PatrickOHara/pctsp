#ifndef __PCTSP_PCTSP__
#define __PCTSP_PCTSP__
/** Exact algorithms for PCTSP */

#include <iostream>
#include "scip/message_default.h"

#include "branching.hh"
#include "constraint.hh"
#include "cost_cover.hh"
#include "cycle_cover.hh"
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
        CostNumberType cost_of_edge = cost_map[edge];
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

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> solvePrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<PCTSPedge>& heuristic_edges,
    EdgeCostMap& cost_map,
    VertexPrizeMap& prize_map,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name
);

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> solvePrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& heuristic_edges,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name
);

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> solvePrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<PCTSPedge>& heuristic_edges,
    EdgeCostMap& cost_map,
    VertexPrizeMap& prize_map,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    int branching_max_depth = -1,
    unsigned int branching_strategy = BranchingStrategy::STRONG,
    bool cost_cover_disjoint_paths = false,
    bool cost_cover_shortest_path = false,
    bool cycle_cover = false,
    std::vector<int> disjoint_paths_distances = std::vector<int>(),
    std::string name = "pctsp",
    bool sec_disjoint_tour = true,
    double sec_lp_gap_improvement_threshold = 0.01,
    bool sec_maxflow_mincut = true,
    int sec_max_tailing_off_iterations = -1,
    int sec_sepafreq = 1,
    bool simple_rules_only = true,
    std::filesystem::path solver_dir = "./pctsp",
    float time_limit = 14400
);

std::map<PCTSPedge, SCIP_VAR*> modelPrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<PCTSPedge>& solution_edges,
    EdgeCostMap& cost_map,
    VertexPrizeMap& prize_map,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name,
    bool sec_disjoint_tour = true,
    double sec_lp_gap_improvement_threshold = 0.01,
    bool sec_maxflow_mincut = true,
    int sec_max_tailing_off_iterations = -1,
    int sec_sepafreq = 1,
    bool simple_rules_only = true
);

std::map<PCTSPedge, SCIP_VAR*> modelPrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& solution_edges,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name,
    bool sec_disjoint_tour = true,
    double sec_lp_gap_improvement_threshold = 0.01,
    bool sec_maxflow_mincut = true,
    int sec_max_tailing_off_iterations = -1,
    int sec_sepafreq = 1,
    bool simple_rules_only = true
);

#endif