
#include <gtest/gtest.h>
#include "pctsp/data_structures.hh"
#include "fixtures.hh"
#include <boost/graph/filtered_graph.hpp>

typedef GraphFixture DataStructuresFixture;

template <typename EdgeWeightMap>
struct EdgeWeightGreaterThan4 {
    EdgeWeightGreaterThan4(EdgeWeightMap weight) : m_weight(weight) { }

    template <typename Edge>
    bool operator()(const Edge& e) const {
        return get(m_weight, e) > 4;
    }
    EdgeWeightMap m_weight;
};

TEST_P(DataStructuresFixture, testGetConnectedComponentsVector) {
    auto graph = getGraph();
    auto cost_map = getCostMap(graph);
    EdgeWeightGreaterThan4<EdgeCostMap> filter(cost_map);
    boost::filtered_graph<PCTSPgraph, EdgeWeightGreaterThan4<EdgeCostMap> > f_graph(graph, filter);
}

INSTANTIATE_TEST_SUITE_P(
    TestDataStructures,
    DataStructuresFixture,
    ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::GRID8, GraphType::SUURBALLE)
);
