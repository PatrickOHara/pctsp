
#include <time.h>
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
    case GraphType::COMPLETE4:
    case GraphType::COMPLETE5:
    case GraphType::COMPLETE25: {
        edge_vector = getCompleteEdgeVector(getNumVertices());
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
    case GraphType::COMPLETE25: return 25;
    }
}

EdgeCostMap GraphFixture::getCostMap(PCTSPgraph& graph) {
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
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
        break;
    }
    case GraphType::COMPLETE25: {
        // assign a random integer to the cost for each edge
        // std::random_device rd;  //Will be used to obtain a seed for the random number engine
        // std::mt19937 gen; //Standard mersenne_twister_engine seeded with rd()
        // gen.seed(2596);
        // std::linear_congruential_engine gen((unsigned int) 0);
        // std::uniform_int_distribution<> distrib(1, 100);
        for (int i = 0; i < boost::num_vertices(graph); i++) {
            for (int j = i + 1; j < boost::num_vertices(graph); j++) {
                // cost_map[boost::edge(i, j, graph).first] = distrib(gen);
                cost_map[boost::edge(i, j, graph).first] = (i * 7 + j * 13) % 29;
            }
        }
        break;
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

VertexPrizeMap GraphFixture::getPrizeMap(PCTSPgraph& graph) {
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    switch (GetParam()) {
    case GraphType::COMPLETE4:
    case GraphType::COMPLETE5:
    case GraphType::COMPLETE25:
        // the prize is the same as the ID of the vertex
        for (auto vertex: boost::make_iterator_range(boost::vertices(graph))) {
            prize_map[vertex] = vertex;
        }
        break;
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

VertexPrizeMap GraphFixture::getGenOnePrizeMap(PCTSPgraph& graph) {
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    switch (GetParam()) {
    case GraphType::COMPLETE4:
    case GraphType::COMPLETE5:
    case GraphType::COMPLETE25:
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
    case GraphType::COMPLETE25: {
        auto n = getNumVertices();
        return ((n-10) * (n-9)) / 2;   // total prize of the graph
    }
    case GraphType::GRID8:
    case GraphType::SUURBALLE:
        return 6;
    }
}

PCTSPvertex GraphFixture::getRootVertex() {
    auto graph = getGraph();
    return boost::vertex(0, graph);
}

std::string GraphFixture::getParamName() {
    switch (GetParam()) {
        case GraphType::COMPLETE4: return "COMPLETE4"; break;
        case GraphType::COMPLETE5: return "COMPLETE5"; break;
        case GraphType::COMPLETE25: return "COMPLETE25"; break;
        case GraphType::GRID8: return "GRID8"; break;
        case GraphType::SUURBALLE: return "SUURBALLE"; break;
        default: return "UNKNOWN"; break;
    }
}

std::list<PCTSPvertex> GraphFixture::getSmallTour() {
    std::list<PCTSPvertex> small_tour;
    switch (GetParam()) {
        case GraphType::COMPLETE4: 
        case GraphType::COMPLETE5: small_tour = {0, 1, 2, 0}; break;
        case GraphType::GRID8: small_tour = {0, 1, 3, 2, 0}; break;
        case GraphType::SUURBALLE: small_tour = {0, 1, 5, 2, 0}; break;
        case GraphType::COMPLETE25:
            for (PCTSPvertex i = 0; i < 10; i++) small_tour.push_back(i); break;
        default: small_tour = {}; break;
    }
    return small_tour;
}

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> BadlyNamedFixture::getBadlyNamedEdges() {
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>> edges;
    switch (GetParam()) {
        case BadlyNamedEdges::WELL_NAMED: edges = { {0, 1}, {1, 2}, {2, 3} }; break;
        case BadlyNamedEdges::BADLY_NAMED: edges = { {10, 11}, {11, 12}, {12, 13}}; break;
        case BadlyNamedEdges::EMPTY: edges = {}; break;
        case BadlyNamedEdges::REVERSE_NAMED: edges = { {5, 4}, {5, 3}, {4, 3}, {3,2}, {2,1}, {2, 0}}; break;
    }
	return edges;
}

std::map<std::pair<PCTSPvertex, PCTSPvertex>, int> BadlyNamedFixture::getOldCostMap() {
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, int> cost_map;
    auto old_edges = BadlyNamedFixture::getBadlyNamedEdges();
    for (int i = 0; i < old_edges.size(); i ++) {
        cost_map[old_edges[i]] = i;
    }
    return cost_map;
}

std::map<PCTSPvertex, int> BadlyNamedFixture::getOldPrizeMap() {
    std::map<PCTSPvertex, int> old_prize;
    switch (GetParam()) {
        case BadlyNamedEdges::WELL_NAMED: old_prize = {{0, 5}, {1, 4}, {2, 3}, {3, 2}}; break;
        case BadlyNamedEdges::BADLY_NAMED: old_prize = { {10, 1}, {11, 2}, {12, 3}, {13, 4}}; break;
        case BadlyNamedEdges::EMPTY: old_prize = {}; break;
        case BadlyNamedEdges::REVERSE_NAMED: old_prize = { {5, 1}, {4, 2}, {3, 3}, {2, 4}, {1, 5}, {0, 6}}; break;
    }
    return old_prize;
}
