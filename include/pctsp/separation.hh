#ifndef __PCTSP_SEPARATION__
#define __PCTSP_SEPARATION__

#include "graph.hh"
#include <objscip/objscip.h>

std::vector<PCTSPvertex> getSolutionVertices(SCIP* mip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map);
std::vector<PCTSPedge> getSolutionEdges(SCIP* mip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map, bool add_self_loops = false);
PCTSPgraph getSolutionGraph(SCIP* mip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map, bool add_self_loops = false);

#endif