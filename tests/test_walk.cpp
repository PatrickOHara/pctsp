#include <gtest/gtest.h>

#include "fixtures.hh"
#include "pctsp/walk.hh"

using namespace std;

typedef GraphFixture SuurballeGraphFixture;
typedef GraphFixture WalkFixture;

TEST_P(SuurballeGraphFixture, testTotalPrize) {
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    std::list<int> tour = { 0, 1, 3, 6, 7, 2, 0 };
    EXPECT_EQ(total_prize(graph, tour, prize_map), 7);
}

TEST_P(SuurballeGraphFixture, testTotalPrizeOfTour) {
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    std::list<int> tour = { 0, 1, 3, 6, 7, 2, 0 };
    EXPECT_EQ(total_prize_of_tour(graph, tour, prize_map), 6);
}

TEST_P(WalkFixture, testTotalPrize) {
    typedef typename PCTSPgraph::vertex_descriptor Vertex;
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);

    // prize of vertex is one
    std::list<int> tour = { 1, 2, 3, 1 };
    int expected_prize = 4;
    EXPECT_EQ(total_prize(graph, tour, prize_map), expected_prize);

    // total prize of empty tour is zero
    std::list<int> empty_tour = {};
    EXPECT_EQ(total_prize(graph, empty_tour, prize_map), 0);

    // total prize of one vertex is itself
    std::list<int> one_tour = { 1 };
    EXPECT_EQ(total_prize(graph, one_tour, prize_map), 1);
}

TEST_P(SuurballeGraphFixture, testTotalCost) {
    PCTSPgraph graph = getGraph();
    auto cost_map = getCostMap(graph);
    std::list<int> tour = { 0, 1, 3, 6, 7, 2, 0 };

    // expected cost of tour
    int expected_cost = 21;
    EXPECT_EQ(total_cost(graph, tour, cost_map), expected_cost);

    // empty tour expected to have zero cost
    std::list<int> empty_tour = {};
    EXPECT_EQ(total_cost(graph, empty_tour, cost_map), 0);

    // expect edge exception if edge not in graph
    std::list<int> invalid_tour = { 0, 1, 2, 0 };
    EXPECT_THROW(total_cost(graph, invalid_tour, cost_map),
        EdgeNotFoundException);
}

void CheckReorderTourFromRoot(std::list<int>& tour, int root_vertex) {
    std::list<int> new_tour = ReorderTourFromRoot(tour, root_vertex);
    std::list<int>::iterator root_finder =
        std::find(new_tour.begin(), new_tour.end(), root_vertex);
    EXPECT_EQ(std::distance(new_tour.begin(), root_finder), 0);
}

TEST(TestWalk, TestReorderTourFromRoot) {
    typedef typename std::list<int> Tour;

    int root_vertex = 0;
    Tour tour1 = { 1, 2, 3, 0, 1 };
    Tour tour2 = { 0, 1, 2, 3, 0 };
    Tour tour3 = { 0 };
    Tour tour4 = {};
    Tour tour5 = { 0, 0 };

    CheckReorderTourFromRoot(tour1, root_vertex);
    CheckReorderTourFromRoot(tour2, root_vertex);
    CheckReorderTourFromRoot(tour3, root_vertex);
    CheckReorderTourFromRoot(tour4, root_vertex);
    CheckReorderTourFromRoot(tour5, root_vertex);
}

TEST_P(WalkFixture, testGetEdgeVector) {
    auto graph = getGraph();
    typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor Vertex;
    std::vector<Vertex> tour;
    for (int i = 0; i < 4; i++)
        tour.push_back(boost::vertex(i, graph));
    tour.push_back(boost::vertex(0, graph));
    auto start = tour.begin();
    auto last = tour.end();
    auto edge_vector = getEdgesInWalk(graph, start, last);
    EXPECT_EQ(tour.size(), edge_vector.size() + 1);
    for (auto const& edge : edge_vector) {
        auto u = boost::source(edge, graph);
        auto v = boost::target(edge, graph);
        auto it = std::find(tour.begin(), tour.end(), u);
        it++;
        EXPECT_EQ(v, *it);
    }
}

INSTANTIATE_TEST_SUITE_P(
    TestWalk,
    WalkFixture,
    ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5)
);
INSTANTIATE_TEST_SUITE_P(TestWalk, SuurballeGraphFixture,
    ::testing::Values(GraphType::SUURBALLE)
);
