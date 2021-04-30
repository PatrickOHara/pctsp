#include "fixtures.hh"
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include "pctsp/subtour_elimination.hh"

TEST_F(SuurballeGraphFixture, testGetEdgesFromCut) {
    PCTSPgraph graph = get_suurballe_graph();
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);

    BOOST_AUTO(parities, boost::make_one_bit_color_map(num_vertices(graph), get(boost::vertex_index, graph)));
    auto cost_of_cut = stoer_wagner_min_cut(graph, cost_map, boost::parity_map(parities));
    auto edges_in_cut = getEdgesFromCut(graph, parities);
    int total_cost = 0;
    for (auto const& edge : edges_in_cut) {
        total_cost += cost_map[edge];
    }
    EXPECT_EQ(total_cost, cost_of_cut);
}

TEST(TestSubtourElimination, testDisconnectedCut) {
    PCTSPgraph graph;
    add_edge(0, 1, { 1 }, graph);
    add_edge(1, 2, { 1 }, graph);
    add_edge(2, 0, { 1 }, graph);
    add_edge(3, 4, { 1 }, graph);
    add_edge(4, 5, { 1 }, graph);
    add_edge(3, 5, { 1 }, graph);

    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    BOOST_AUTO(parities, boost::make_one_bit_color_map(num_vertices(graph), get(boost::vertex_index, graph)));

    auto cost_of_cut = stoer_wagner_min_cut(graph, cost_map, boost::parity_map(parities));
    EXPECT_EQ(cost_of_cut, 0);
    auto edges_in_cut = getEdgesFromCut(graph, parities);
    EXPECT_EQ(edges_in_cut.size(), 0);
}