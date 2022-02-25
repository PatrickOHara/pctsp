
#include "fixtures.hh"
#include "pctsp/walk.hh"
#include <gtest/gtest.h>

TEST_P(GraphFixture, testGetGraph) {
    auto graph = getGraph();
    switch (GetParam()) {
    case GraphType::COMPLETE4: {
        EXPECT_EQ(boost::num_vertices(graph), 4);
        break;
    }
    case GraphType::COMPLETE5: {
        EXPECT_EQ(boost::num_vertices(graph), 5);
        break;
    }
    case GraphType::COMPLETE25: {     
        auto n = boost::num_vertices(graph);   
        EXPECT_EQ(n, 25);
        auto prize_map = getPrizeMap(graph);
        EXPECT_EQ(totalPrizeOfGraph(graph, prize_map), (n * (n-1)) / 2);
        break;
    }
    case GraphType::GRID8: {
        auto cost_map = getCostMap(graph);
        EXPECT_EQ(boost::num_vertices(graph), 8);
        EXPECT_EQ(boost::num_edges(graph), 10);
        EXPECT_EQ(cost_map[boost::edge(0, 1, graph).first], 1);
        EXPECT_EQ(cost_map[boost::edge(1, 4, graph).first], 5);
        break;
    }
    case GraphType::SUURBALLE: {
        EXPECT_EQ(boost::num_vertices(graph), 8);
        EXPECT_EQ(boost::num_edges(graph), 12);
        auto cost_map = getCostMap(graph);
        EXPECT_EQ(cost_map[boost::edge(0, 1, graph).first], 3);
    }
    }
}

INSTANTIATE_TEST_SUITE_P(TestFixtures, GraphFixture, ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::GRID8, GraphType::SUURBALLE, GraphType::COMPLETE25));
