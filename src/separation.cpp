#include "pctsp/separation.hh"

void removeIsolatedVertices(PCTSPgraph& graph) {
    std::list<PCTSPvertex> remove;
    for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
        // isolated vertices have degree zero
        if (boost::degree(vertex, graph) == 0) {
            remove.push_back(vertex);
        }
    }
    for (auto const& vertex : remove) {
        boost::remove_vertex(vertex, graph);
    }
}
