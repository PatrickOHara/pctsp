/** Tests for cycle cover inequalities */

#include <gtest/gtest.h>

#include "fixtures.hh"
#include "pctsp/algorithms.hh"
#include "pctsp/cycle_cover.hh"

typedef GraphFixture CycleCoverFixture;

TEST_P(CycleCoverFixture, testCycleCover) {
    auto graph = getGraph();
    auto prize_map = getGenOnePrizeMap(graph);  // prize is 1 on each vertex
    auto cost_map = getCostMap(graph);
    auto root_vertex = getRootVertex();
    auto test_case = GetParam();
    int quota = getQuota();
    std::string name = "testCycleCover" + getParamName();
    std::filesystem::path logs_dir = ".logs";

    // assign zero cost to self loops
    addSelfLoopsToGraph(graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    // initialise empty model
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    std::vector<PCTSPedge> heuristic_edges = {};

    solvePrizeCollectingTSP(scip, graph, heuristic_edges, cost_map, prize_map, quota, root_vertex, -1, BranchingStrategy::RELPSCOST, false, false, true, {}, name, false, 0.01, false, -1, 1, logs_dir, 60);

    int num_expected_cc_conss;
    int num_actual_cc_conss = getNumCycleCoverCutsAdded(scip);
    double opt_value = SCIPsolGetOrigObj(SCIPgetBestSol(scip));
    double expected_opt;
    switch (test_case) {
        case GraphType::GRID8: {
            // only 1 cycle cover added to vertices {0,1,2,3}
            // this is sufficient to solve the grid8 problem optimally
            num_expected_cc_conss = 1;
            expected_opt = 14;
            break;
        }
        case GraphType::SUURBALLE: {
            num_expected_cc_conss = 0;
            expected_opt = 20;
            break;
        }
        case GraphType::COMPLETE4: {
            num_expected_cc_conss = 0;
            expected_opt = 10;
            break;
        }
        case GraphType::COMPLETE5: {
            num_expected_cc_conss = 0;
            expected_opt = 12;
            break;
        }
        default: {
            num_expected_cc_conss = 0;
            expected_opt = 0;
            break;
        }
    }
    EXPECT_EQ(expected_opt, opt_value);
    EXPECT_EQ(num_expected_cc_conss, num_actual_cc_conss);

    // remember to free memory
    SCIPfree(&scip);
}

INSTANTIATE_TEST_SUITE_P(
    TestCycleCover,
    CycleCoverFixture,
    ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::GRID8, GraphType::SUURBALLE)
);