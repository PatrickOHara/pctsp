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
    std::string name = "testPathCostCover";

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
        default: {
            quota = getQuota();
            tour = {};
            break;
        }
    }
    // get the edges in the initial tour
    auto first = tour.begin();
    auto last = tour.end();
    std::vector<PCTSPedge> solution_edges = getEdgesInWalk(graph, first, last);

    // initialise empty model
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);

    // setup the solver with minimal add ons
    std::vector<NodeStats> node_stats;  // save node statistics
    std::map<PCTSPedge, SCIP_VAR*> edge_variable_map;
    std::map<PCTSPedge, int> weight_map;
    ProbDataPCTSP* probdata = new ProbDataPCTSP(&graph, &root_vertex, &edge_variable_map, &quota, &node_stats);
    SCIPcreateObjProb(
        scip,
        name.c_str(),
        probdata,
        true
    );
    includeShortestPathCostCover(scip, graph, cost_map, root_vertex);

    // add custom message handler
    SCIP_MESSAGEHDLR* message_handler;
    std::string log_filepath = ".logs/" + name + ".txt";
    SCIPcreateMessagehdlrDefault(&message_handler, false, log_filepath.c_str(), true);
    SCIPsetMessagehdlr(scip, message_handler);

    // move prizes of vertices onto the weight of an edge
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // add variables and constraints to SCIP model
    PCTSPmodelWithoutSECs(scip, graph, cost_map, weight_map, quota, root_vertex, edge_variable_map);

    SCIPsetIntParam(scip, "presolving/maxrounds", 0);

    // add selected heuristics to reduce the upper bound on the optimal
    if (solution_edges.size() > 0) {
        auto first = solution_edges.begin();
        auto last = solution_edges.end();
        SCIP_HEUR* heur = NULL;
        addHeuristicEdgesToSolver(scip, graph, heur, edge_variable_map, first, last);
    }
    // solve the model
    SCIPsolve(scip);

    // count the number of shortest path cost cover inequalities added
    CostCoverEventHandler* cc_handler = dynamic_cast<CostCoverEventHandler*>(SCIPfindObjEventhdlr(scip, SHORTEST_PATH_COST_COVER_NAME.c_str()));
    int num_expected_cc_conss;
    int num_actual_cc_conss = cc_handler->getNumConssAdded();
    auto opt_value = SCIPsolGetOrigObj(SCIPgetBestSol(scip));
    switch (test_case) {
        case GraphType::GRID8: {
            // vertices 4, 5, 6, 7 are all above the cost limit
            num_expected_cc_conss = 4;
            break;
        }
        case GraphType::SUURBALLE: {
            // vertex 6 (f) is expected to be above the cost limit
            num_expected_cc_conss = 1;
            EXPECT_EQ(opt_value, 15);
            break;
        }
        default: {
            // on the complete graphs, no vertices are expected to be above the cost limit
            num_expected_cc_conss = 0;
            break;
        }
    }
    EXPECT_EQ(num_expected_cc_conss, num_actual_cc_conss);

    // remember to free memory
    SCIPfree(&scip);
}

INSTANTIATE_TEST_SUITE_P(
    TestCostCover,
    CostCoverFixture,
    ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::GRID8, GraphType::SUURBALLE)
);
