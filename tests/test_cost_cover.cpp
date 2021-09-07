/** Tests for cost cover inequalities */

#include <gtest/gtest.h>

#include "fixtures.hh"
#include "pctsp/cost_cover.hh"


typedef GraphFixture CostCoverFixture;

TEST_P(CostCoverFixture, testShortestPath) {
    PCTSPgraph graph = getGraph();
    PCTSPvertex source_vertex = getRootVertex();
    std::vector<PCTSPvertex> pred (boost::num_vertices(graph));
    std::vector<int> distances (boost::num_vertices(graph));
    auto vindex = get(boost::vertex_index, graph);
    auto pred_map = boost::predecessor_map(boost::make_iterator_property_map(pred.begin(), vindex));
    auto distance_map = boost::distance_map(boost::make_iterator_property_map(distances.begin(), vindex));
    auto weight_map = get(edge_weight, graph);
    for (auto edge: boost::make_iterator_range(boost::edges(graph))) {
        weight_map[edge] = 1;
    }
    dijkstra_shortest_paths(
        graph,
        source_vertex,
        pred_map.distance_map(boost::make_iterator_property_map(
            distances.begin(), vindex
        ))
    );
    for (auto vertex: boost::make_iterator_range(boost::vertices(graph))) {
        if (vertex != source_vertex) {
            EXPECT_NE(distances[vertex], 0);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    TestCostCover,
    CostCoverFixture,
    ::testing::Values(GraphType::GRID8, GraphType::SUURBALLE)
);
