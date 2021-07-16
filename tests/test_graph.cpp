/** Test getting the adjacency grap*/

#include "pctsp/graph.hh"
#include "fixtures.hh"
#include <gtest/gtest.h>

typedef GraphFixture GraphFix;

TEST_P(GraphFix, testGetVertexPairVectorFromEdgeSubset) {
    PCTSPgraph graph = getGraph();
    std::vector<PCTSPedge> edge_subset;
    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        if (boost::source(edge, graph) == 0)
            edge_subset.push_back(edge);
        if (boost::target(edge, graph) == 0)
            edge_subset.push_back(edge);
    }
    auto edge_vector = getVertexPairVectorFromEdgeSubset(graph, edge_subset);
    EXPECT_GE(edge_vector.size(), 2);
    for (auto const& pair : edge_vector) {
        EXPECT_TRUE(pair.first == 0 || pair.second == 0);
    }
}

TEST(TestGraph, testGetVerticesOfEdges) {
    PCTSPgraph graph;
    PCTSPedge edge1 = boost::add_edge(0, 1, graph).first;
    PCTSPedge edge2 = boost::add_edge(1, 2, graph).first;
    PCTSPedge edge3 = boost::add_edge(2, 3, graph).first;

    std::vector<PCTSPedge> edges;
    edges.push_back(edge1);
    edges.push_back(edge2);

    auto first = edges.begin();
    auto last = edges.end();
    auto vertices = getVerticesOfEdges(graph, first, last);
    std::cout << std::endl;
    EXPECT_FALSE(std::find(vertices.begin(), vertices.end(), 0) == vertices.end());
    EXPECT_FALSE(std::find(vertices.begin(), vertices.end(), boost::vertex(1, graph)) == vertices.end());
    EXPECT_TRUE(std::find(vertices.begin(), vertices.end(), 3) == vertices.end());
}

TEST(TestGraph, testEdgesFromVertexPairs) {
    PCTSPgraph graph;
    PCTSPedge edge1 = boost::add_edge(0, 1, graph).first;
    PCTSPedge edge2 = boost::add_edge(1, 2, graph).first;
    PCTSPedge edge3 = boost::add_edge(2, 3, graph).first;

    typedef typename std::pair<int, int> Pair;
    std::vector<Pair> pairs;
    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        int u = boost::source(edge, graph);
        int v = boost::target(edge, graph);
        Pair pair(u, v);
        pairs.push_back(pair);
    }

    auto first = pairs.begin();
    auto last = pairs.end();
    auto edge_vector = edgesFromVertexPairs(graph, first, last);
    EXPECT_EQ(edge_vector.size(), pairs.size());
    EXPECT_EQ(std::count(edge_vector.begin(), edge_vector.end(), edge1), 1);
    EXPECT_EQ(std::count(edge_vector.begin(), edge_vector.end(), edge2), 1);
    EXPECT_EQ(std::count(edge_vector.begin(), edge_vector.end(), edge3), 1);
}

TEST_P(GraphFix, testGetSelfLoops) {
    PCTSPgraph graph = getGraph();
    typedef boost::graph_traits< PCTSPgraph >::vertex_descriptor Vertex;
    std::vector<Vertex> subset = { 0, 1, 3 };
    for (auto const vertex : subset) {
        boost::add_edge(vertex, vertex, graph);
    }
    auto first = subset.begin();
    auto last = subset.end();
    auto self_loops = getSelfLoops(graph, first, last);
    for (auto const edge : self_loops) {
        Vertex source = boost::source(edge, graph);
        EXPECT_EQ(source, boost::target(edge, graph));
        EXPECT_FALSE(std::find(subset.begin(), subset.end(), source) == subset.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
    TestGraph,
    GraphFix,
    ::testing::Values(GraphType::GRID8, GraphType::SUURBALLE)
);
