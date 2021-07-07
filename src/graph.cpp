#include "pctsp/graph.hh"

using namespace std;


std::vector <PCTSPedge> getEdgeVectorOfGraph(PCTSPgraph& graph) {
    std::vector < PCTSPedge> edge_vector;
    int i = 0;
    for (PCTSPedge edge : boost::make_iterator_range(boost::edges(graph))) {
        edge_vector.insert(edge_vector.begin() + i, edge);
        i++;
    }
    return edge_vector;
}

VertexPairVector getVertexPairVectorFromEdgeSubset(
    PCTSPgraph& graph,
    std::vector < PCTSPedge> edge_subset_vector
) {
    VertexPairVector edges;
    for (auto const& edge : edge_subset_vector) {
        PCTSPvertex s = (PCTSPvertex)boost::source(edge, graph);
        PCTSPvertex t = (PCTSPvertex)boost::target(edge, graph);
        VertexPair e = { s, t };
        edges.push_back(e);
    }
    return edges;
}
