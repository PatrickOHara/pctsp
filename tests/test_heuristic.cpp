#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <typeinfo>

#include "fixtures.hh"
#include "pctsp/heuristic.hh"

using namespace std;
using namespace boost;

int expected_num_edges_in_complete_graph(int n_vertices) {
    return (n_vertices * (n_vertices - 1)) / 2;
}

TEST(TestExpandCollapse, test_unitary_gain) {
    EXPECT_EQ(unitary_gain(10, 2, 2, 2), 5);
    EXPECT_EQ(unitary_gain(2, 0, 1, 3), 0.5);
    // triangle inequality does not hold
    EXPECT_EQ(unitary_gain(10, 15, 5, 5), -2);
}

TEST(TestExpandCollapse, test_calculate_average_gain) {
    typedef std::map<int, UnitaryGainOfVertex> UnitaryGainMap;
    UnitaryGainMap gain_map;

    std::unordered_set<int> vertices_in_tour = {0, 1, 2, 3, 0};
    EXPECT_EQ(vertices_in_tour.size(), 4);

    // vertices not in the tour
    int a = 4;
    int b = 5;
    int c = 6;

    // initialise unitary gain and gain map
    UnitaryGainOfVertex gain_a = UnitaryGainOfVertex();
    UnitaryGainOfVertex gain_b = UnitaryGainOfVertex();
    UnitaryGainOfVertex gain_c = UnitaryGainOfVertex();

    gain_a.gain_of_vertex = 1;
    gain_a.index_of_extension = 1;
    gain_b.gain_of_vertex = 2;
    gain_b.index_of_extension = 2;
    gain_c.gain_of_vertex = 6;
    gain_c.index_of_extension = 3;

    gain_map[a] = gain_a;
    gain_map[b] = gain_b;
    gain_map[c] = gain_c;

    // expect average gain to be 4
    EXPECT_EQ(calculate_average_gain(vertices_in_tour, gain_map), 3);

    // now add a to the tour and re-calcuate avg gain (should not include gain
    // of a)
    vertices_in_tour.insert(a);
    EXPECT_EQ(calculate_average_gain(vertices_in_tour, gain_map), 4);
}

TEST_P(CompleteGraphParameterizedFixture, test_unitary_gain_of_vertex) {
    typedef typename PCTSPgraph::vertex_descriptor Vertex;
    PCTSPgraph g = get_complete_PCTSPgraph();
    Vertex v0 = boost::vertex(0, g);
    Vertex v1 = boost::vertex(1, g);
    Vertex v2 = boost::vertex(2, g);
    // std::list<Vertex> tour = {v0, v1, v2, v0};
    std::list<int> tour = {0, 1, 2, 0};
    auto prize_map = get(&PCTSPvertexProperties::prize, g);
    auto cost_map = get(&PCTSPedgeProperties::cost, g);
    // Vertex missing_vertex = boost::vertex(3, g);
    int missing_vertex = 3;
    UnitaryGainOfVertex gain =
        unitary_gain_of_vertex(g, tour, cost_map, prize_map, missing_vertex);
    if (num_vertices(g) == 4) {
        EXPECT_FLOAT_EQ(0.5, gain.gain_of_vertex);
    } else if (num_vertices(g) == 5) {
        EXPECT_FLOAT_EQ(3.0 / 7.0, gain.gain_of_vertex);
    }
}

TEST_P(CompleteGraphParameterizedFixture, test_extend) {
    typedef typename PCTSPgraph::vertex_descriptor Vertex;
    PCTSPgraph g = get_complete_PCTSPgraph();

    EXPECT_EQ(expected_num_edges_in_complete_graph(num_vertices(g)),
              num_edges(g));
    EXPECT_EQ(1, g[1].prize);
    PCTSPgraph::edge_descriptor e0 = *out_edges(0, g).first;
    Vertex v0 = boost::vertex(0, g);
    Vertex v1 = boost::vertex(1, g);
    Vertex v2 = boost::vertex(2, g);
    PCTSPgraph::edge_descriptor e1 = edge(v0, v2, g).first;
    EXPECT_EQ(0, g[e0].cost);
    EXPECT_EQ(1, g[e1].cost);

    // get the edge property map from bundled internal property
    auto prize_map = get(&PCTSPvertexProperties::prize, g);
    auto cost_map = get(&PCTSPedgeProperties::cost, g);
    int prize0 = get(prize_map, 0);
    ASSERT_EQ(prize0, 0);
    int cost0 = get(cost_map, e0);
    EXPECT_EQ(g[e0].cost, cost0);

    // other parameters
    int root = 0;
    int quota = 2;

    // std::list<Vertex> tour = {v0, v1, v2, v0};
    std::list<int> tour = {0, 1, 2, 0};
    extend(g, tour, cost_map, prize_map);
    EXPECT_EQ(tour.size(), num_vertices(g));
}

TEST_F(SuurballeGraphFixture, test_extend) {
    PCTSPgraph graph = get_suurballe_graph();
    std::list<int> tour = {0, 1, 3, 6, 7, 2, 0};
    int tour_size_before_extend = tour.size();

    // get the edge property map from bundled internal property
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    extend(graph, tour, cost_map, prize_map);

    // we expect that vertex e (index 5) has been added, but not vertex d
    EXPECT_EQ(tour.size(), tour_size_before_extend + 1);
    int d = 4;
    int e = 5;

    // check vertex d not in tour
    auto d_iterator = std::find(tour.begin(), tour.end(), d);
    EXPECT_TRUE(d_iterator == tour.end());

    // check e is added to the tour in the correct index
    auto e_iterator = std::find(tour.begin(), tour.end(), e);
    int e_index = std::distance(tour.begin(), e_iterator);
    EXPECT_FALSE(e_iterator == tour.end());
    EXPECT_EQ(e_index, 5);
}

TEST_F(SuurballeGraphFixture, test_extend_until_prize_feasible) {
    PCTSPgraph graph = get_suurballe_graph();
    std::list<int> tour = {0, 1, 5, 2, 0};
    int quota = 5;
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);

    // run the algorithm
    extend_until_prize_feasible(graph, tour, cost_map, prize_map, quota);

    // we expect the total prize of the tour to be equal to the quota
    EXPECT_EQ(total_prize_of_tour(graph, tour, prize_map), quota);

    // expect vertex g (7) to be in the tour at index 3
    int g = 7;
    auto g_iterator = std::find(tour.begin(), tour.end(), g);
    int g_index = std::distance(tour.begin(), g_iterator);
    EXPECT_FALSE(g_iterator == tour.end());
    EXPECT_EQ(g_index, 3);
}

TEST_F(SuurballeGraphFixture, test_extend_until_prize_feasible_seg_fault) {
    PCTSPgraph graph = get_suurballe_graph();
    std::list<int> tour = {0, 4, 1, 3, 6, 7, 5, 2, 0};
    int quota = 10;
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    extend_until_prize_feasible(graph, tour, cost_map, prize_map, quota);
}

TEST_F(SuurballeGraphFixture, testCollapse) {
    // get graphs with property maps
    PCTSPgraph graph = get_suurballe_graph();
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);

    // this tour can be collapsed to {0, 1, 5, 2, 0}
    std::list<int> tour = {0, 1, 4, 6, 7, 2, 0};
    int quota = 5;
    int root_vertex = 0;

    // run the collapse heuristic
    std::list<int> new_tour =
        collapse(graph, tour, cost_map, prize_map, quota, root_vertex);
    EXPECT_EQ(total_cost(graph, new_tour, cost_map), 16);
    EXPECT_GE(total_prize_of_tour(graph, new_tour, prize_map), quota);
    EXPECT_GT(new_tour.size(), quota);

    // ensure the first vertex of the tour is the root vertex
    auto first_root_it =
        std::find(new_tour.begin(), new_tour.end(), root_vertex);
    int root_index = std::distance(new_tour.begin(), first_root_it);
    EXPECT_FALSE(first_root_it == new_tour.end());
    EXPECT_EQ(root_index, 0);

    // ensure the last vertex of the tour is the root vertex
    ++first_root_it;
    auto second_root_it = std::find(first_root_it, tour.end(), root_vertex);
    int second_root_index = std::distance(new_tour.begin(), second_root_it);
    EXPECT_FALSE(second_root_it == new_tour.end());
    EXPECT_EQ(second_root_index, new_tour.size() - 1);
}

TEST_P(CompleteGraphParameterizedFixture, testCollapse) {
    PCTSPgraph graph = get_complete_PCTSPgraph();
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);

    // test the tour that is returned is the same as the input tour
    std::list<int> tour = {0, 1, 2, 0};
    int root_vertex = 0;
    int quota = 4;
    std::list<int> same_tour =
        collapse(graph, tour, cost_map, prize_map, quota, root_vertex);
    ASSERT_THAT(same_tour, testing::ElementsAre(0, 1, 2, 0));

    if (boost::num_vertices(graph) == 5) {
        tour = {0, 1, 2, 3, 4, 0};
        quota = 5;
        std::list<int> new_tour =
            collapse(graph, tour, cost_map, prize_map, quota, root_vertex);
        EXPECT_EQ(total_prize_of_tour(graph, new_tour, prize_map), 5);
        EXPECT_EQ(total_cost(graph, new_tour, cost_map), 9);
    }
}

TEST(TestExpandCollapse, testIndexOfReverseIterator) {
    std::list<int> mylist = {0, 1, 2, 3, 4, 0};
    auto rit = mylist.rbegin();
    EXPECT_EQ(indexOfReverseIterator(mylist, rit), 5);
    ++rit;
    ++rit;
    EXPECT_EQ(indexOfReverseIterator(mylist, rit), 3);
    ++rit;
    ++rit;
    ++rit;
    EXPECT_EQ(indexOfReverseIterator(mylist, rit), 0);
}

INSTANTIATE_TEST_SUITE_P(TestExpandCollapse, CompleteGraphParameterizedFixture,
                        ::testing::Values(4, 5));
