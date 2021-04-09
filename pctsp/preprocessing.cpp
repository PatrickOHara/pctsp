#include "pctsp/preprocessing.hh"

void assignZeroCostToSelfLoops(PCTSPgraph &graph, PCTSPcostMap &cost_map) {
    for (auto vertex : boost::make_iterator_range(vertices(graph))) {
        auto edge = boost::edge(vertex, vertex, graph);
        if (edge.second) {
            cost_map[edge.first] = 0;
        }
    }
}

void addSelfLoopsToGraph(PCTSPgraph &graph) {
    for (auto vertex : boost::make_iterator_range(vertices(graph))) {
        boost::add_edge(vertex, vertex, graph);
    }
}