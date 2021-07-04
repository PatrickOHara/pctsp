#ifndef __PCTSP_POSTPROCESSING__
#define __PCTSP_POSTPROCESSING__

#include "graph.hh"

using namespace scip;


std::vector<PCTSPvertex> getSolutionVertices(SCIP* mip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map);
std::vector<PCTSPedge> getSolutionEdges(
    SCIP* mip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map,
    bool add_self_loops = false
);
void getSolutionGraph(
    SCIP* mip,
    PCTSPgraph& graph,
    PCTSPgraph& solution_graph,
    SCIP_SOL* sol,
    std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map,
    bool add_self_loops = false
);

void logSolutionEdges(
    SCIP* mip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    PCTSPedgeVariableMap& edge_variable_map
);

// lookup table from support vertex to input vertex
typedef std::map<PCTSPvertex, PCTSPvertex> SupportToInputVertexLookup;

SupportToInputVertexLookup getSupportToInputVertexLookup(std::vector<PCTSPvertex>& input_vertices);

SupportToInputVertexLookup getSupportToInputVertexLookupFromEdges(
    StdEdgeVector& input_edge_vector
);

#endif