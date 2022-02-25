#ifndef __PCTSP_POSTPROCESSING__
#define __PCTSP_POSTPROCESSING__

#include <boost/graph/filtered_graph.hpp>

#include "graph.hh"

using namespace scip;


std::vector<PCTSPvertex> getSolutionVertices(SCIP* scip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map);
std::vector<PCTSPedge> getSolutionEdges(
    SCIP* scip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map,
    bool add_self_loops = false
);
void getSolutionGraph(
    SCIP* scip,
    PCTSPgraph& graph,
    PCTSPgraph& solution_graph,
    SCIP_SOL* sol,
    std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map,
    bool add_self_loops = false
);

bool isVarPositive(SCIP* scip, SCIP_SOL* sol, SCIP_VAR* var);

bool isSolutionIntegral(SCIP* scip, SCIP_SOL* sol);

template <typename TEdgeVariableMap>
struct PositiveEdgeVarFilter {

    SCIP* scip;
    SCIP_SOL* sol;
    TEdgeVariableMap* edge_var_map;

    PositiveEdgeVarFilter() {}

    PositiveEdgeVarFilter(
        SCIP* scip,
        SCIP_SOL* sol,
        TEdgeVariableMap* edge_var_map
    ) : scip(scip), sol(sol), edge_var_map(edge_var_map) {}

    // return true if the edge variable is positive
    template <typename TEdge>
    bool operator()(const TEdge& e) const {
        auto var = (*edge_var_map)[e];
        return isVarPositive(scip, sol, var);
    }
};

template <typename TGraph, typename TEdgeVariableMap>
boost::filtered_graph<TGraph, PositiveEdgeVarFilter<TEdgeVariableMap>> filterGraphByPositiveEdgeVars(
    SCIP* scip,
    TGraph& graph,
    SCIP_SOL* sol,
    TEdgeVariableMap& edge_var_map
) {

    typedef PositiveEdgeVarFilter<TEdgeVariableMap> EdgeFilter;
    typedef typename boost::filtered_graph<TGraph, EdgeFilter> FilteredGraph;

    EdgeFilter filter (scip, sol, &edge_var_map);
    FilteredGraph f_graph (graph, filter);
    return f_graph;
}

void logSolutionEdges(
    SCIP* scip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    PCTSPedgeVariableMap& edge_variable_map
);



#endif