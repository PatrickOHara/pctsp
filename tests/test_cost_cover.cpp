/** Tests for cost cover inequalities */

#include <gtest/gtest.h>

#include "fixtures.hh"
#include "pctsp/algorithms.hh"
#include "pctsp/cost_cover.hh"


typedef GraphFixture CostCoverFixture;

TEST_P(CostCoverFixture, testPathCostCover) {
    auto graph = getGraph();
    auto prize_map = getGenOnePrizeMap(graph);  // prize is 1 on each vertex
    auto cost_map = getCostMap(graph);
    auto root_vertex = getRootVertex();
    auto test_case = GetParam();
    int quota;
    std::list<PCTSPvertex> tour;

    // assign zero cost to self loops
    addSelfLoopsToGraph(graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    // quota and tour depend on the graph
    switch (test_case) {
        case GraphType::GRID8: {
            quota = 4;
            tour = {0, 1, 4, 5, 3, 2, 0};
            break;
        }
        case GraphType::SUURBALLE: {
            quota = 3;
            tour = {0, 1, 5, 2, 0};
            break;
        }
    }
    // get the edges in the initial tour
    auto first = tour.begin();
    auto last = tour.end();
    std::vector<PCTSPedge> solution_edges = getEdgesInWalk(graph, first, last);

    // run the solver
    PCTSPbranchAndCut(graph, solution_edges, cost_map, prize_map, quota, root_vertex);
}

INSTANTIATE_TEST_SUITE_P(
    TestCostCover,
    CostCoverFixture,
    ::testing::Values(GraphType::GRID8, GraphType::SUURBALLE)
);
