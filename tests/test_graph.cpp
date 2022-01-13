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

TEST_P(GraphFix, testGetEdgesInducedByVertices) {
    auto graph = getGraph();
    std::vector<PCTSPvertex> inducing_vertices = {0, 1, 2, 3};
    auto induced_edges = getEdgesInducedByVertices(graph, inducing_vertices);
    int expected_num_edges;
    switch (GetParam()) {
        case GraphType::COMPLETE4:
            expected_num_edges = boost::num_edges(graph);
            break;
        case GraphType::COMPLETE5: {
            expected_num_edges = boost::num_edges(graph) - 4;
            break;
        }
        case GraphType::GRID8: {
            expected_num_edges = 4;
            break;
        }
        case GraphType::SUURBALLE: {
            expected_num_edges = 3;
            break;
        }
        default: {
            expected_num_edges = 0;
            break;
        }
    }
    EXPECT_EQ(induced_edges.size(), expected_num_edges);
}

TEST(TestGraph, testGetSubpathOfCycle) {
    std::list<PCTSPvertex> cycle1 = {0, 1, 2, 0};
    std::list<PCTSPvertex> cycle2 = {0, 0};

    int first1a = 1;
    int last1a = 3;
    auto path1a = getSubpathOfCycle(cycle1, first1a, last1a);
    EXPECT_EQ(path1a.size(), 3);
    EXPECT_EQ(path1a[0], 1);
    EXPECT_EQ(path1a[1], 2);
    EXPECT_EQ(path1a[2], 0);

    int first1b = 2;
    int last1b = 1;
    auto path1b = getSubpathOfCycle(cycle1, first1b, last1b);
    EXPECT_EQ(path1b.size(), 3);
    EXPECT_EQ(path1b[0], 2);
    EXPECT_EQ(path1b[1], 0);
    EXPECT_EQ(path1b[2], 1);
}

TEST_P(GraphFix, testDepthFirstSearch) {
    auto graph = getGraph();
    auto root = getRootVertex();

    std::vector<bool> marked (boost::num_vertices(graph));
    std::vector<PCTSPvertex> parent (boost::num_vertices(graph));

    depthFirstSearch(graph, root, marked, parent, 2);

    std::vector<bool> expected_marked;

    switch (GetParam()) {
        case GraphType::GRID8: {
            expected_marked = {true, true, true, true, true, false, false, false};
            break;
        }
        case GraphType::SUURBALLE: {
            expected_marked = {true, true, true, true, true, true, false, true};
            break;
        }
        default: {
            expected_marked = std::vector<bool>(boost::num_vertices(graph));
            for (auto vertex: boost::make_iterator_range(boost::vertices(graph)))
                expected_marked[vertex] = true;
            break;
        }
    }
    for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
        EXPECT_EQ(marked[vertex], expected_marked[vertex]);
    }
}

TEST_P(GraphFix, testBreadthFirstSearch) {
    auto graph = getGraph();
    auto root = getRootVertex();

    std::vector<bool> marked (boost::num_vertices(graph));
    std::vector<PCTSPvertex> parent (boost::num_vertices(graph));

    breadthFirstSearch(graph, root, marked, parent, 2);

    std::vector<bool> expected_marked;

    switch (GetParam()) {
        case GraphType::GRID8: {
            expected_marked = {true, true, true, true, true, false, false, false};
            break;
        }
        default: {
            expected_marked = std::vector<bool>(boost::num_vertices(graph));
            for (auto vertex: boost::make_iterator_range(boost::vertices(graph)))
                expected_marked[vertex] = true;
            break;
        }
    }
    for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
        EXPECT_EQ(marked[vertex], expected_marked[vertex]);
    }
}

TEST_P(GraphFix, testPathInTreeFromParents) {
    auto graph = getGraph();
    auto root = getRootVertex();

    std::vector<bool> marked (boost::num_vertices(graph));
    std::vector<PCTSPvertex> parent (boost::num_vertices(graph));

    breadthFirstSearch(graph, root, marked, parent, 2);
    PCTSPvertex target;
    std::list<PCTSPvertex> expected_path;

    switch (GetParam()) {
        case GraphType::GRID8: {
            target = 4;
            expected_path = {0, 1, 4};
            break;
        }
        case GraphType::SUURBALLE: {
            target = 6;
            expected_path = {0, 4, 6};
            break;
        }
        default: {
            target = 1;
            expected_path = {0, 1};
            break;
        }
    }
    std::list<PCTSPvertex> actual_path = pathInTreeFromParents(parent, root, target);
    EXPECT_EQ(expected_path, actual_path);
}

INSTANTIATE_TEST_SUITE_P(
    TestGraph,
    GraphFix,
    ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::GRID8, GraphType::SUURBALLE)
);
