#include "pctsp/preprocessing.hh"

void addSelfLoopsToGraph(PCTSPgraph& graph) {
    for (auto vertex : boost::make_iterator_range(vertices(graph))) {
        boost::add_edge(vertex, vertex, graph);
    }
}

