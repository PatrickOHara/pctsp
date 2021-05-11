
#include "fixtures.hh"
#include <gtest/gtest.h>

TEST_P(GraphFixture, testGetGraph) {
    switch (GetParam()) {
    case GraphType::COMPLETE4: {
        auto graph = getGraph();
        EXPECT_EQ(boost::num_vertices(graph), 4);
        break;
    }
    case GraphType::COMPLETE5: {
        auto graph = getGraph();
        EXPECT_EQ(boost::num_vertices(graph), 5);
        break;
    }
    case GraphType::GRID8: {
        auto graph = getGraph();
        auto cost_map = getCostMap(graph);
        EXPECT_EQ(boost::num_vertices(graph), 8);
        EXPECT_EQ(boost::num_edges(graph), 10);
        EXPECT_EQ(cost_map[boost::edge(0, 1, graph).first], 1);
        EXPECT_EQ(cost_map[boost::edge(1, 4, graph).first], 5);
        break;
    }
    case GraphType::SUURBALLE: {
        auto graph = getGraph();
        EXPECT_EQ(boost::num_vertices(graph), 8);
        EXPECT_EQ(boost::num_edges(graph), 12);
        auto cost_map = getCostMap(graph);
        EXPECT_EQ(cost_map[boost::edge(0, 1, graph).first], 3);
    }
    }
}

INSTANTIATE_TEST_SUITE_P(TestFixtures, GraphFixture, ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::GRID8, GraphType::SUURBALLE));
