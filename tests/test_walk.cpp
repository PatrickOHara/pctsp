#include <gtest/gtest.h>

#include "fixtures.hh"
#include "pctsp/walk.hh"

using namespace std;

TEST_F(SuurballeGraphFixture, testTotalPrize) {
    PCTSPgraph graph = SuurballeGraphFixture::get_suurballe_graph();
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);
    std::list<int> tour = {0, 1, 3, 6, 7, 2, 0};
    EXPECT_EQ(total_prize(graph, tour, prize_map), 7);
}

TEST_F(SuurballeGraphFixture, testTotalPrizeOfTour) {
    PCTSPgraph graph = SuurballeGraphFixture::get_suurballe_graph();
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);
    std::list<int> tour = {0, 1, 3, 6, 7, 2, 0};
    EXPECT_EQ(total_prize_of_tour(graph, tour, prize_map), 6);
}

TEST_P(CompleteGraphParameterizedFixture, testTotalPrize) {
    typedef typename PCTSPgraph::vertex_descriptor Vertex;
    PCTSPgraph graph = get_complete_PCTSPgraph();
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);

    // prize of vertex is equal to its label
    std::list<int> tour = {1, 2, 3, 1};
    int expected_prize = 1 + 2 + 3 + 1;
    EXPECT_EQ(total_prize(graph, tour, prize_map), expected_prize);

    // total prize of empty tour is zero
    std::list<int> empty_tour = {};
    EXPECT_EQ(total_prize(graph, empty_tour, prize_map), 0);

    // total prize of one vertex is itself
    std::list<int> one_tour = {1};
    EXPECT_EQ(total_prize(graph, one_tour, prize_map), 1);
}

TEST_F(SuurballeGraphFixture, testTotalCost) {
    PCTSPgraph graph = SuurballeGraphFixture::get_suurballe_graph();
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    std::list<int> tour = {0, 1, 3, 6, 7, 2, 0};

    // expected cost of tour
    int expected_cost = 21;
    EXPECT_EQ(total_cost(graph, tour, cost_map), expected_cost);

    // empty tour expected to have zero cost
    std::list<int> empty_tour = {};
    EXPECT_EQ(total_cost(graph, empty_tour, cost_map), 0);

    // expect edge exception if edge not in graph
    std::list<int> invalid_tour = {0, 1, 2, 0};
    EXPECT_THROW(total_cost(graph, invalid_tour, cost_map),
                 EdgeNotFoundException);
}

void CheckReorderTourFromRoot(std::list<int> &tour, int root_vertex) {
    std::list<int> new_tour = ReorderTourFromRoot(tour, root_vertex);
    std::list<int>::iterator root_finder =
        std::find(new_tour.begin(), new_tour.end(), root_vertex);
    EXPECT_EQ(std::distance(new_tour.begin(), root_finder), 0);
}

TEST(TestWalk, TestReorderTourFromRoot) {
    typedef typename std::list<int> Tour;

    int root_vertex = 0;
    Tour tour1 = {1, 2, 3, 0, 1};
    Tour tour2 = {0, 1, 2, 3, 0};
    Tour tour3 = {0};
    Tour tour4 = {};
    Tour tour5 = {0, 0};

    CheckReorderTourFromRoot(tour1, root_vertex);
    CheckReorderTourFromRoot(tour2, root_vertex);
    CheckReorderTourFromRoot(tour3, root_vertex);
    CheckReorderTourFromRoot(tour4, root_vertex);
    CheckReorderTourFromRoot(tour5, root_vertex);
}
