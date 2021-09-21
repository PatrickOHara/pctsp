
#include <gtest/gtest.h>
#include "pctsp/data_structures.hh"
#include "fixtures.hh"
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/connected_components.hpp>

typedef GraphFixture DataStructuresFixture;

struct EdgeWeightGreaterThan4 {

    EdgeCostMap m_weight;

    EdgeWeightGreaterThan4() {}
    EdgeWeightGreaterThan4(EdgeCostMap& weight) : m_weight(weight) { }

    bool operator()(const PCTSPedge& e) const {
        return get(m_weight, e) > 4;
    }
};

boost::filtered_graph<PCTSPgraph, EdgeWeightGreaterThan4> getFgraph(PCTSPgraph& graph, EdgeCostMap& cost_map) {
    typedef typename boost::filtered_graph<PCTSPgraph, EdgeWeightGreaterThan4> FGraph;
    EdgeWeightGreaterThan4 filter(cost_map);
    FGraph f_graph(graph, filter);
    return f_graph;
}

TEST_P(DataStructuresFixture, testGetConnectedComponentsVector) {
    auto graph = getGraph();
    auto cost_map = getCostMap(graph);
    auto f_graph = getFgraph(graph, cost_map);
    std::vector< int > component(boost::num_vertices(f_graph));
    int n_components = boost::connected_components(f_graph, &component[0]);
    auto component_vectors = getConnectedComponentsVectors(f_graph, n_components, component);

    int expected_n_components;
    switch (GetParam()) {
        case GraphType::GRID8: expected_n_components = 6; break;
        case GraphType::SUURBALLE: expected_n_components = 3; break;
        case GraphType::COMPLETE4: expected_n_components = 3; break;
        case GraphType::COMPLETE5: expected_n_components = 2; break;
        default: expected_n_components = 1;
    }
    EXPECT_EQ(expected_n_components, n_components);
}

INSTANTIATE_TEST_SUITE_P(
    TestDataStructures,
    DataStructuresFixture,
    ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::GRID8, GraphType::SUURBALLE)
);
