
#include "fixtures.hh"

std::vector<std::pair<int, int>> getCompleteEdgeVector(int n_vertices) {
    std::vector<std::pair<int, int>> edge_vector;
    for (int i = 0; i < n_vertices; i++) {
        for (int j = i + 1; j < n_vertices; j++) {
            edge_vector.push_back(std::pair(i, j));
        }
    }
    return edge_vector;
}

PCTSPgraph GraphFixture::getGraph() {
    PCTSPgraph graph;
    for (auto const& edge : getEdgeVector()) {
        boost::add_edge(edge.first, edge.second, graph);
    }
    return graph;
}

std::vector<std::pair<int, int>> GraphFixture::getEdgeVector() {
    std::vector<std::pair<int, int>> edge_vector;
    auto test_case = GetParam();
    switch (test_case) {
    case GraphType::COMPLETE4: {
        edge_vector = getCompleteEdgeVector(4);
        break;
    }
    case GraphType::COMPLETE5: {
        edge_vector = getCompleteEdgeVector(5);
        break;
    }
    case GraphType::GRID8: {
        edge_vector.push_back(std::pair(0, 1));
        edge_vector.push_back(std::pair(0, 2));
        edge_vector.push_back(std::pair(1, 3));
        edge_vector.push_back(std::pair(1, 4));
        edge_vector.push_back(std::pair(2, 3));
        edge_vector.push_back(std::pair(3, 5));
        edge_vector.push_back(std::pair(4, 5));
        edge_vector.push_back(std::pair(4, 6));
        edge_vector.push_back(std::pair(5, 7));
        edge_vector.push_back(std::pair(6, 7));
        break;
    }
    case GraphType::SUURBALLE: {
        edge_vector.push_back(std::pair(0, 1));
        edge_vector.push_back(std::pair(0, 2));
        edge_vector.push_back(std::pair(0, 4));
        edge_vector.push_back(std::pair(1, 3));
        edge_vector.push_back(std::pair(1, 4));
        edge_vector.push_back(std::pair(1, 5));
        edge_vector.push_back(std::pair(2, 5));
        edge_vector.push_back(std::pair(2, 7));
        edge_vector.push_back(std::pair(3, 6));
        edge_vector.push_back(std::pair(4, 6));
        edge_vector.push_back(std::pair(5, 7));
        edge_vector.push_back(std::pair(6, 7));
        break;
    }
    }
    return edge_vector;
}

int GraphFixture::getNumVertices() {
    switch (GetParam()) {
    case GraphType::COMPLETE4: return 4;
    case GraphType::COMPLETE5: return 5;
    case GraphType::GRID8: return 8;
    case GraphType::SUURBALLE: return 8;
    }
}

PCTSPcostMap GraphFixture::getCostMap(PCTSPgraph& graph) {
    PCTSPcostMap cost_map;
    switch (GetParam()) {
    case GraphType::COMPLETE4:
    case GraphType::COMPLETE5: {
        int e = 0;
        for (int i = 0; i < boost::num_vertices(graph); i++) {
            for (int j = i + 1; j < boost::num_vertices(graph); j++) {
                cost_map[boost::edge(i, j, graph).first] = e;
                e++;
            }
        }
    }
    case GraphType::GRID8: {
        // assign uniform cost to every edge
        for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
            auto source = boost::source(edge, graph);
            auto target = boost::target(edge, graph);
            if (((source == 1) & (target == 4)) | ((source == 3) & (target == 5)))
                cost_map[edge] = 5;     // make two heavy edges in the middle
            else
                cost_map[edge] = 1;
        }
        break;
    }
    case GraphType::SUURBALLE: {
        cost_map[boost::edge(0, 1, graph).first] = 3;
        cost_map[boost::edge(0, 2, graph).first] = 2;
        cost_map[boost::edge(0, 4, graph).first] = 8;
        cost_map[boost::edge(1, 3, graph).first] = 1;
        cost_map[boost::edge(1, 4, graph).first] = 4;
        cost_map[boost::edge(1, 5, graph).first] = 6;
        cost_map[boost::edge(2, 5, graph).first] = 5;
        cost_map[boost::edge(2, 7, graph).first] = 3;
        cost_map[boost::edge(3, 6, graph).first] = 5;
        cost_map[boost::edge(4, 6, graph).first] = 1;
        cost_map[boost::edge(5, 7, graph).first] = 2;
        cost_map[boost::edge(6, 7, graph).first] = 7;
        break;
    }
    }
    return cost_map;
}

PCTSPprizeMap GraphFixture::getPrizeMap(PCTSPgraph& graph) {
    PCTSPprizeMap prize_map;
    switch (GetParam()) {
    case GraphType::COMPLETE4:
    case GraphType::COMPLETE5:
    case GraphType::GRID8:
    case GraphType::SUURBALLE: {
        // assign uniform prize to every vertex
        for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
            prize_map[vertex] = 1;
        }
        break;
    }
    }
    return prize_map;
}

std::vector<int> GraphFixture::getVertexVector() {
    std::vector<int> vertex_vector;
    for (int i = 0; i < getNumVertices(); i++)
        vertex_vector.push_back(i);
    return vertex_vector;
}

int GraphFixture::getQuota() {
    switch (GetParam()) {
    case GraphType::COMPLETE4:
    case GraphType::COMPLETE5:
        return 4;
    case GraphType::GRID8:
    case GraphType::SUURBALLE:
        return 6;
    }
}

// a complete graph over N vertices
PCTSPgraph CompleteGraphParameterizedFixture::get_complete_PCTSPgraph() {
    PCTSPgraph graph;
    int N = GetParam();
    add_vertices_with_prizes(graph, N);
    add_edges_with_costs(graph);
    return graph;
};

// add vertices to complete graph - prize is index
void CompleteGraphParameterizedFixture::add_vertices_with_prizes(PCTSPgraph& g,
    int N) {
    for (int i = 0; i < N; i++) {
        boost::add_vertex({ i }, g);
    }
};

// cost of an edge is the same as its index
void CompleteGraphParameterizedFixture::add_edges_with_costs(PCTSPgraph& g) {
    int N = num_vertices(g);
    int e = 0;
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            // Graph::edge_descriptor e = edge (i, j, g).first;
            add_edge(i, j, { e }, g);
            e++;
        }
    }
};

// a graph from Suurballe's original paper
PCTSPgraph SuurballeGraphFixture::get_suurballe_graph() {
    // all vertices have uniform prize
    PCTSPgraph graph;
    typedef typename PCTSPgraph::vertex_descriptor Vertex;
    Vertex s = boost::add_vertex({ 1 }, graph);
    Vertex a = boost::add_vertex({ 1 }, graph);
    Vertex b = boost::add_vertex({ 1 }, graph);
    Vertex c = boost::add_vertex({ 1 }, graph);
    Vertex d = boost::add_vertex({ 1 }, graph);
    Vertex e = boost::add_vertex({ 1 }, graph);
    Vertex f = boost::add_vertex({ 1 }, graph);
    Vertex g = boost::add_vertex({ 1 }, graph);
    boost::add_edge(s, a, { 3 }, graph);
    boost::add_edge(s, b, { 2 }, graph);
    boost::add_edge(s, d, { 8 }, graph);
    boost::add_edge(a, c, { 1 }, graph);
    boost::add_edge(a, d, { 4 }, graph);
    boost::add_edge(a, e, { 6 }, graph);
    boost::add_edge(b, e, { 5 }, graph);
    boost::add_edge(b, g, { 3 }, graph);
    boost::add_edge(c, f, { 5 }, graph);
    boost::add_edge(d, f, { 1 }, graph);
    boost::add_edge(e, g, { 2 }, graph);
    boost::add_edge(f, g, { 7 }, graph);
    return graph;
};
