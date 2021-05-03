#ifndef __PCTSP_SEPARATION__
#define __PCTSP_SEPARATION__

#include "graph.hh"
#include <boost/graph/connected_components.hpp>
#include <objscip/objscip.h>

std::vector<PCTSPvertex> getSolutionVertices(SCIP* mip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map);
std::vector<PCTSPedge> getSolutionEdges(SCIP* mip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map, bool add_self_loops = false);
PCTSPgraph getSolutionGraph(SCIP* mip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map, bool add_self_loops = false);

/** Returns true if the graph is a simple cycle */
template <class UndirectedGraph>
bool isGraphSimpleCycle(UndirectedGraph& support_graph) {
    if (boost::num_vertices(support_graph) == 0) return false;
    if (boost::num_edges(support_graph) == 0) return false;
    std::vector< int > component(boost::num_vertices(support_graph));
    int n_components = boost::connected_components(support_graph, &component[0]);
    if (n_components != 1) return false;    // a simple cycle must be connected

    for (auto vertex : boost::make_iterator_range(boost::vertices(support_graph))) {
        if (boost::degree(vertex, support_graph) != 2) return false;
    }
    return true;
}

#endif