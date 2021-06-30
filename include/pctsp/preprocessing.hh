#ifndef __PCTSP_PREPROCESSING__
#define __PCTSP_PREPROCESSING__

/** Functions for preprocessing graphs and data structures. */

#include "pctsp/graph.hh"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

using namespace boost;

/** Assign zero cost to all edge self loops in the graph. Insert into map. */
template <typename UndirectedGraph, typename CostMap>
void assignZeroCostToSelfLoops(UndirectedGraph& graph, CostMap& cost_map) {
    for (auto vertex : boost::make_iterator_range(vertices(graph))) {
        auto edge = boost::edge(vertex, vertex, graph);
        if (edge.second) {
            cost_map[edge.first] = 0;
        }
    }
}

/** Given a simple graph, add self-loops onto all vertices */
void addSelfLoopsToGraph(PCTSPgraph& graph);

/** Returns true if there exists at least one self loops on all vertices.
 * False otherwise.
 */
template <typename Graph> bool hasSelfLoopsOnAllVertices(Graph& graph) {
    for (auto vertex : make_iterator_range(vertices(graph))) {
        if (edge(vertex, vertex, graph).second == false) {
            return false;
        }
    }
    return true;
}

template <typename Graph, typename PrizeMap, typename WeightMap>
void putPrizeOntoEdgeWeights(Graph& graph, PrizeMap& prize_map,
    WeightMap& weight_map) {

    typename graph_traits<Graph>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei) {
        auto source_vertex = source(*ei, graph);
        int weight_of_edge = 0;
        auto target_vertex = target(*ei, graph);
        if (source_vertex == target_vertex) {
            // if the edge is a self loop on a vertex, then get the prize of
            // the vertex assign the weight of the self-loop to be the prize
            // of the vertex
            weight_of_edge = prize_map[source_vertex];
        }
        auto e = edge(source_vertex, target_vertex, graph);
        weight_map[e.first] = weight_of_edge;
    }
}
#endif