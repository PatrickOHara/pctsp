#ifndef __PCTSP_SUBTOUR_ELIMINATION__
#define __PCTSP_SUBTOUR_ELIMINATION__

#include "graph.hh"
#include <objscip/objscip.h>

template <class UndirectedGraph, class ParityMap>
std::vector<typename boost::graph_traits<UndirectedGraph>::edge_descriptor> getEdgesFromCut(UndirectedGraph& graph, ParityMap& parity_map) {
    typedef typename boost::graph_traits<UndirectedGraph>::edge_descriptor Edge;
    std::vector<Edge> edges;
    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        // get edges where the endpoints lie in different cut sets
        auto source = boost::source(edge, graph);
        auto target = boost::target(edge, graph);
        if (parity_map[source] != parity_map[target]) {
            edges.push_back(edge);
        }
    }
    return edges;
}

#endif