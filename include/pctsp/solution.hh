#ifndef __PCTSP_POSTPROCESSING__
#define __PCTSP_POSTPROCESSING__

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

void logSolutionEdges(
    SCIP* scip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    PCTSPedgeVariableMap& edge_variable_map
);



#endif