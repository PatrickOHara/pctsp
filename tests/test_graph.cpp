/** Test getting the adjacency grap*/

#include "pctsp/graph.hh"
#include "fixtures.hh"
#include <gtest/gtest.h>

typedef GraphFixture EdgeSubsetGraph;

TEST_P(EdgeSubsetGraph, testGetVertexPairVectorFromEdgeSubset) {
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

INSTANTIATE_TEST_SUITE_P(
    TestGraph,
    EdgeSubsetGraph,
    ::testing::Values(GraphType::GRID8, GraphType::SUURBALLE)
);
