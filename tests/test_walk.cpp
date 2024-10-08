#include <gtest/gtest.h>

#include "fixtures.hh"
#include "pctsp/graph.hh"
#include "pctsp/walk.hh"

using namespace std;

typedef GraphFixture SuurballeGraphFixture;
typedef GraphFixture WalkFixture;

TEST_P(SuurballeGraphFixture, testTotalPrize) {
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    std::list<int> tour = { 0, 1, 3, 6, 7, 2, 0 };  // include prize of root twice
    EXPECT_EQ(totalPrize(prize_map, tour), 7);
}

TEST_P(SuurballeGraphFixture, testTotalPrizeOfTour) {
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    std::list<PCTSPvertex> tour = { 1, 3, 6, 7, 2, 0, 1 };
    EXPECT_EQ(totalPrizeOfTour(prize_map, tour), 6);
}

TEST_P(WalkFixture, testTotalPrize) {
    typedef typename PCTSPgraph::vertex_descriptor Vertex;
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);

    // prize of vertex is equal to its ID
    std::list<int> walk = { 1, 2, 3};
    int expected_prize = 1 + 2 + 3;
    EXPECT_EQ(totalPrize(prize_map, walk), expected_prize);

    // total prize of empty tour is zero
    std::list<int> empty_tour = {};
    EXPECT_EQ(totalPrize(prize_map, empty_tour), 0);

    // total prize of one vertex is itself
    std::list<int> one_tour = { 1 };
    EXPECT_EQ(totalPrize(prize_map, one_tour), 1);
}

TEST_P(SuurballeGraphFixture, testTotalCost) {
    PCTSPgraph graph = getGraph();
    auto cost_map = getCostMap(graph);
    std::list<PCTSPvertex> tour = { 0, 1, 3, 6, 7, 2, 0 };

    // expected cost of tour
    int expected_cost = 21;
    EXPECT_EQ(totalCost(graph, tour, cost_map), expected_cost);

    // empty tour expected to have zero cost
    std::list<PCTSPvertex> empty_tour = {};
    EXPECT_EQ(totalCost(graph, empty_tour, cost_map), 0);

    // expect edge exception if edge not in graph
    std::list<PCTSPvertex> invalid_tour = { 0, 1, 2, 0 };
    EXPECT_THROW(totalCost(graph, invalid_tour, cost_map),
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

TEST(TestWalk, testIsInternalVertexOfWalk) {
    std::vector<int> walk = { 0, 1, 2, 3, 0 };
    int zero = 0;
    int one = 1;
    int four = 4;
    EXPECT_TRUE(isInternalVertexOfWalk(walk, one));
    EXPECT_FALSE(isInternalVertexOfWalk(walk, zero));
    EXPECT_FALSE(isInternalVertexOfWalk(walk, four));
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

TEST_P(SuurballeGraphFixture, testShortestPathBlacklist) {
    auto graph = getGraph();
    auto cost_map = getCostMap(graph);
    PCTSPvertex source = 0;
    PCTSPvertex target =  7;
    int n = boost::num_vertices(graph);
    std::vector<bool> mark (n);
    std::vector<PCTSPvertex> predecessor(n);
    std::vector<int> distance(n);
    
    std::vector<ColorType> color_vector (n);
    mark[2] = true;

    try {
        dijkstraShortestPathBlacklist(graph, source, target, predecessor, distance, cost_map, color_vector, mark);
    }
    catch (TargetVertexFound) {

    }
        auto path_st = pathInTreeFromParents(predecessor, source, target);
        default_color_type b = default_color_type::black_color;
        EXPECT_EQ(color_vector[2], b);
        EXPECT_EQ(distance[target], 11);
}

INSTANTIATE_TEST_SUITE_P(
    TestWalk,
    WalkFixture,
    ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5)
);
INSTANTIATE_TEST_SUITE_P(TestWalk, SuurballeGraphFixture,
    ::testing::Values(GraphType::SUURBALLE)
);
