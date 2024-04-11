#include <gtest/gtest.h>
#include <objscip/objscipdefplugins.h>
#include "pctsp/graph.hh"
#include "pctsp/robust.hh"
#include "fixtures.hh"


typedef GraphFixture RobustFixture;

TEST_P(RobustFixture, testSolveRobustDistRobustPrizeCollectingTsp) {
    auto graph = getGraph();
    auto quota = getQuota();
    auto rootVertex = getRootVertex();
    auto costMeanMap = getCostMap(graph);
    auto costSigmaMap = getCostSigmaMap(graph);
    auto prizeMap = getPrizeMap(graph);
    // addSelfLoopsToGraph(graph);
    // assignZeroCostToSelfLoops(graph, costMeanMap);
    // assignZeroCostToSelfLoops(graph, costSigmaMap);

    std::string testName = "testSolveRobustDistRobustPrizeCollectingTsp-";
    testName.append(getParamName());

    std::vector<PCTSPvertex> tour;
    auto first = tour.begin();
    auto last = tour.end();
    std::vector<PCTSPedge> heuristicEdges = getEdgesInWalk(graph, first, last);

    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);    // FIXME!!!!
    auto solution = solveDistRobustPrizeCollectingTsp(
        scip,
        graph,
        costMeanMap,
        costSigmaMap,
        prizeMap,
        quota,
        rootVertex,
        testName
    );
}

INSTANTIATE_TEST_SUITE_P(
    TestAlgorithms,
    RobustFixture,
    ::testing::Values(GraphType::SUURBALLE, GraphType::GRID8, GraphType::COMPLETE4, GraphType::COMPLETE5)
);
