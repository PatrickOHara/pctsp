
#include "fixtures.hh"

// a complete graph over N vertices
PCTSPgraph CompleteGraphParameterizedFixture::get_complete_PCTSPgraph() {
    PCTSPgraph graph;
    int N = GetParam();
    add_vertices_with_prizes(graph, N);
    add_edges_with_costs(graph);
    return graph;
};

// add vertices to complete graph - prize is index
void CompleteGraphParameterizedFixture::add_vertices_with_prizes(PCTSPgraph &g,
                                                                 int N) {
    for (int i = 0; i < N; i++) {
        boost::add_vertex({i}, g);
    }
};

// cost of an edge is the same as its index
void CompleteGraphParameterizedFixture::add_edges_with_costs(PCTSPgraph &g) {
    int N = num_vertices(g);
    int e = 0;
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            // Graph::edge_descriptor e = edge (i, j, g).first;
            add_edge(i, j, {e}, g);
            e++;
        }
    }
};

// a graph from Suurballe's original paper
PCTSPgraph SuurballeGraphFixture::get_suurballe_graph() {
    // all vertices have uniform prize
    PCTSPgraph graph;
    typedef typename PCTSPgraph::vertex_descriptor Vertex;
    Vertex s = boost::add_vertex({1}, graph);
    Vertex a = boost::add_vertex({1}, graph);
    Vertex b = boost::add_vertex({1}, graph);
    Vertex c = boost::add_vertex({1}, graph);
    Vertex d = boost::add_vertex({1}, graph);
    Vertex e = boost::add_vertex({1}, graph);
    Vertex f = boost::add_vertex({1}, graph);
    Vertex g = boost::add_vertex({1}, graph);
    boost::add_edge(s, a, {3}, graph);
    boost::add_edge(s, b, {2}, graph);
    boost::add_edge(s, d, {8}, graph);
    boost::add_edge(a, c, {1}, graph);
    boost::add_edge(a, d, {4}, graph);
    boost::add_edge(a, e, {6}, graph);
    boost::add_edge(b, e, {5}, graph);
    boost::add_edge(b, g, {3}, graph);
    boost::add_edge(c, f, {5}, graph);
    boost::add_edge(d, f, {1}, graph);
    boost::add_edge(e, g, {2}, graph);
    boost::add_edge(f, g, {7}, graph);
    return graph;
};
