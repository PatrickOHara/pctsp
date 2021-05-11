#ifndef __PCTSP_SEPARATION__
#define __PCTSP_SEPARATION__

#include "graph.hh"
#include "solution.hh"
#include <boost/graph/connected_components.hpp>
#include <objscip/objscip.h>

/** Returns true if the graph is a simple cycle */
template <class UndirectedGraph>
bool isGraphSimpleCycle(UndirectedGraph& support_graph, std::vector<int>& component_vector) {
    if (boost::num_vertices(support_graph) == 0) return false;
    if (boost::num_edges(support_graph) == 0) return false;
    int n_components = boost::connected_components(support_graph, &component_vector[0]);
    if (n_components != 1) return false;    // a simple cycle must be connected

    for (auto vertex : boost::make_iterator_range(boost::vertices(support_graph))) {
        if (boost::degree(vertex, support_graph) != 2) return false;
    }
    return true;
}

#endif