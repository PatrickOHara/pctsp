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

std::vector<SCIP_VAR*> getEdgeVariables(
    SCIP* scip,
    PCTSPgraph& graph,
    PCTSPedgeVariableMap& edge_variable_map,
    std::vector<PCTSPedge>& edges
) {
    auto first = edges.begin();
    auto last = edges.end();
    return getEdgeVariables(scip, graph, edge_variable_map, first, last);
}

std::vector<PCTSPedge> getEdgesInducedByVertices(PCTSPgraph& graph, std::vector<PCTSPvertex>& vertices) {
    auto first = vertices.begin();
    auto last = vertices.end();
    return getEdgesInducedByVertices(graph, first, last);
}

std::vector<PCTSPedge> getSelfLoops(PCTSPgraph& graph, std::vector<PCTSPvertex>& vertices) {
    auto first = vertices.begin();
    auto last = vertices.end();
    return getSelfLoops(graph, first, last);
}
