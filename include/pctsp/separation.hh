#ifndef __PCTSP_SEPARATION__
#define __PCTSP_SEPARATION__

#include "graph.hh"
#include "solution.hh"
#include <boost/graph/connected_components.hpp>
#include <objscip/objscip.h>

bool isSimpleCycle(PCTSPgraph& graph, std::vector<PCTSPedge>& edge_vector);

/** Returns true if the graph is a simple cycle */
bool isGraphSimpleCycle(PCTSPgraph& graph, std::vector<int>& component_vector);

std::vector<PCTSPedge> getEdgeVectorOfGraph(PCTSPgraph& graph);

#endif