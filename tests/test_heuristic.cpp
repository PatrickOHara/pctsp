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

typedef GraphFixture CompleteGraphParameterizedFixture;
typedef GraphFixture SuurballeGraphFixture;
typedef GraphFixture ExtensionFixture;

TEST(TestExpandCollapse, testUnitaryGain) {
    EXPECT_EQ(unitary_gain(10, 2, 2, 2), 5);
    EXPECT_EQ(unitary_gain(2, 0, 1, 3), 0.5);
    // triangle inequality does not hold
    EXPECT_EQ(unitary_gain(10, 15, 5, 5), -2);
}

TEST(TestExpandCollapse, testCalculateAverageGain) {
    typedef std::map<int, ExtensionVertex> UnitaryGainMap;
    UnitaryGainMap gain_map;

    std::unordered_set<int> vertices_in_tour = { 0, 1, 2, 3, 0 };
    EXPECT_EQ(vertices_in_tour.size(), 4);

    // vertices not in the tour
    int a = 4;
    int b = 5;
    int c = 6;

    // initialise unitary gain and gain map
    ExtensionVertex gain_a = ExtensionVertex();
    ExtensionVertex gain_b = ExtensionVertex();
    ExtensionVertex gain_c = ExtensionVertex();

    gain_a.value = 1;
    gain_a.index = 1;
    gain_b.value = 2;
    gain_b.index = 2;
    gain_c.value = 6;
    gain_c.index = 3;

    gain_map[a] = gain_a;
    gain_map[b] = gain_b;
    gain_map[c] = gain_c;

    // expect average gain to be 4
    EXPECT_EQ(calculateAverageGain(vertices_in_tour, gain_map), 3);

    // now add a to the tour and re-calcuate avg gain (should not include gain
    // of a)
    vertices_in_tour.insert(a);
    EXPECT_EQ(calculateAverageGain(vertices_in_tour, gain_map), 4);
}

TEST_P(CompleteGraphParameterizedFixture, testUnitaryGainOfVertex) {
    typedef typename PCTSPgraph::vertex_descriptor Vertex;
    PCTSPgraph g = getGraph();
    Vertex v0 = boost::vertex(0, g);
    Vertex v1 = boost::vertex(1, g);
    Vertex v2 = boost::vertex(2, g);
    // std::list<Vertex> tour = {v0, v1, v2, v0};
    std::list<PCTSPvertex> tour = { 0, 1, 2, 0 };
    auto prize_map = getPrizeMap(g);
    auto cost_map = getCostMap(g);
    // Vertex missing_vertex = boost::vertex(3, g);
    int missing_vertex = 3;
    ExtensionVertex gain =
        unitaryGainOfVertex(g, tour, cost_map, prize_map, missing_vertex);
    if (num_vertices(g) == 4) {
        EXPECT_FLOAT_EQ(0.5, gain.value);
    }
    else if (num_vertices(g) == 5) {
        EXPECT_FLOAT_EQ(3.0 / 7.0, gain.value);
    }
}

TEST_P(CompleteGraphParameterizedFixture, test_extend) {
    typedef typename PCTSPgraph::vertex_descriptor Vertex;
    PCTSPgraph g = getGraph();
    auto prize_map = getPrizeMap(g);
    auto cost_map = getCostMap(g);

    EXPECT_EQ(expected_num_edges_in_complete_graph(num_vertices(g)),
        num_edges(g));
    EXPECT_EQ(1, prize_map[1]);
    PCTSPgraph::edge_descriptor e0 = *out_edges(0, g).first;
    Vertex v0 = boost::vertex(0, g);
    Vertex v1 = boost::vertex(1, g);
    Vertex v2 = boost::vertex(2, g);
    PCTSPgraph::edge_descriptor e1 = edge(v0, v2, g).first;
    EXPECT_EQ(0, cost_map[e0]);
    EXPECT_EQ(1, cost_map[e1]);

    // get the edge property map from bundled internal property
    int prize0 = get(prize_map, 0);
    ASSERT_EQ(prize0, 0);
    int cost0 = get(cost_map, e0);
    EXPECT_EQ(cost_map[e0], cost0);

    // other parameters
    int root = 0;
    int quota = 2;

    std::list<Vertex> tour = { 0, 1, 2, 0 };
    extend(g, tour, cost_map, prize_map);
    EXPECT_EQ(tour.size(), num_vertices(g));
}

TEST_P(SuurballeGraphFixture, test_extend) {
    PCTSPgraph graph = getGraph();
    std::list<PCTSPvertex> tour = { 0, 1, 3, 6, 7, 2, 0 };
    int tour_size_before_extend = tour.size();

    // get the edge property map from bundled internal property
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);
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

TEST_P(SuurballeGraphFixture, testExtendUntilPrizeFeasible) {
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);
    std::list<PCTSPvertex> tour = { 0, 1, 5, 2, 0 };
    int quota = 5;

    // run the algorithm
    extendUntilPrizeFeasible(graph, tour, cost_map, prize_map, quota);

    // we expect the total prize of the tour to be equal to the quota
    EXPECT_EQ(total_prize_of_tour(graph, tour, prize_map), quota);

    // expect vertex g (7) to be in the tour at index 3
    int g = 7;
    auto g_iterator = std::find(tour.begin(), tour.end(), g);
    int g_index = std::distance(tour.begin(), g_iterator);
    EXPECT_FALSE(g_iterator == tour.end());
    EXPECT_EQ(g_index, 3);
}

TEST_P(SuurballeGraphFixture, testExtendUntilPrizeFeasibleSegFault) {
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);
    std::list<PCTSPvertex> tour = { 0, 4, 1, 3, 6, 7, 5, 2, 0 };
    int quota = 10;
    extendUntilPrizeFeasible(graph, tour, cost_map, prize_map, quota);
}

TEST_P(SuurballeGraphFixture, testCollapse) {
    // get graphs with property maps
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);

    // this tour can be collapsed to {0, 1, 5, 2, 0}
    std::list<PCTSPvertex> tour = { 0, 1, 4, 6, 7, 2, 0 };
    int quota = 5;
    PCTSPvertex root_vertex = 0;

    // run the collapse heuristic
    std::list<PCTSPvertex> new_tour =
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
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);

    // test the tour that is returned is the same as the input tour
    std::list<PCTSPvertex> tour = { 0, 1, 2, 0 };
    int root_vertex = 0;
    int quota = 4;
    std::list<PCTSPvertex> same_tour =
        collapse(graph, tour, cost_map, prize_map, quota, root_vertex);
    ASSERT_THAT(same_tour, testing::ElementsAre(0, 1, 2, 0));

    if (boost::num_vertices(graph) == 5) {
        tour = { 0, 1, 2, 3, 4, 0 };
        quota = 5;
        std::list<PCTSPvertex> new_tour =
            collapse(graph, tour, cost_map, prize_map, quota, root_vertex);
        EXPECT_EQ(total_prize_of_tour(graph, new_tour, prize_map), 5);
        EXPECT_EQ(total_cost(graph, new_tour, cost_map), 9);
    }
}

TEST(TestExpandCollapse, testIndexOfReverseIterator) {
    std::list<int> mylist = { 0, 1, 2, 3, 4, 0 };
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

TEST_P(ExtensionFixture, testNeighborIntersection) {
    PCTSPgraph graph = getGraph();
    auto u = boost::vertex(0, graph);
    auto v = boost::vertex(1, graph);
    auto intersection = neighborIntersection(graph, u, v);
    int expected_size;
    switch (GetParam()) {
        case GraphType::COMPLETE4:
            expected_size = 2;
            break;
        case GraphType::COMPLETE5: {
            expected_size = 3;
            break;
        }
        case GraphType::GRID8: {
            expected_size = 0;
            break;
        }
        case GraphType::SUURBALLE: {
            expected_size = 1;
            break;
        }
        default: {
            expected_size = 0;
            break;
        }
    }
    EXPECT_EQ(expected_size, intersection.size());
}

TEST_P(ExtensionFixture, testExtension) {
    PCTSPgraph graph = getGraph();
    auto cost_map = getCostMap(graph);
    auto prize_map = getPrizeMap(graph);
    auto small_tour = getSmallTour();
    auto old_size = small_tour.size();
    int expected_size;
    extension(graph, small_tour, cost_map, prize_map);
    switch (GetParam()) {
        case GraphType::COMPLETE5:
            expected_size = old_size + 1; break;
        default:
            expected_size = old_size;
    }
    EXPECT_EQ(small_tour.size(), expected_size);
    std::cout << getParamName() << ": ";
    for (auto v: small_tour) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

TEST_P(ExtensionFixture, testSwapPathsInTour) {
    auto small_tour = getSmallTour();
    auto old_length = small_tour.size();
    std::list<PCTSPvertex> new_path = {5, 8, 0, 10};
    // when i < j is the easy test
    int i = 1;
    int j = 3;
    swapPathsInTour(small_tour, new_path, i, j);
    EXPECT_EQ(old_length - (j - i + 1) + new_path.size(), small_tour.size());
}


TEST_P(ExtensionFixture, testSwapPathsInTourWithRoot) {
    auto small_tour = getSmallTour();
    auto old_length = small_tour.size();
    std::list<PCTSPvertex> new_path = {5, 8, 0, 10};
    // when i > j is more difficult
    int i = 3;
    int j = 1;
    swapPathsInTour(small_tour, new_path, i, j);
    EXPECT_EQ(old_length + new_path.size() + 1 - (old_length - i + j + 1), small_tour.size());
    std::cout << getParamName() << ": ";
    for (auto v: small_tour) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

INSTANTIATE_TEST_SUITE_P(TestExpandCollapse, CompleteGraphParameterizedFixture,
    ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5)
);

INSTANTIATE_TEST_SUITE_P(TestExpandCollapse, SuurballeGraphFixture,
    ::testing::Values(GraphType::SUURBALLE)
);

INSTANTIATE_TEST_SUITE_P(TestExtension, ExtensionFixture,
    ::testing::Values(GraphType::SUURBALLE, GraphType::GRID8, GraphType::COMPLETE4, GraphType::COMPLETE5)
);
