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

    // assign zero cost to self loops
    addSelfLoopsToGraph(graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    // initialise empty model
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);

    // setup the solver with minimal add ons
    std::map<PCTSPedge, SCIP_VAR*> edge_variable_map;
    std::map<PCTSPedge, int> weight_map;
    ProbDataPCTSP* probdata = new ProbDataPCTSP(&graph, &root_vertex, &edge_variable_map, &quota);
    SCIPcreateObjProb(
        scip,
        name.c_str(),
        probdata,
        true
    );
    // add cycle cover constraint handler
    auto cycle_cover_conshdlr = new CycleCoverConshdlr(scip);
    SCIPincludeObjConshdlr(scip, cycle_cover_conshdlr, true);

    // add custom message handler
    SCIP_MESSAGEHDLR* message_handler;
    std::string log_filepath = ".logs/" + name + ".txt";
    SCIPcreateMessagehdlrDefault(&message_handler, false, log_filepath.c_str(), true);
    SCIPsetMessagehdlr(scip, message_handler);

    // move prizes of vertices onto the weight of an edge
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // add variables and constraints to SCIP model
    PCTSPmodelWithoutSECs(scip, graph, cost_map, weight_map, quota, root_vertex, edge_variable_map);

    // add basic cycle cover constraint
    SCIP_CONS* cons;
    createBasicCycleCoverCons(scip, &cons);
    SCIPaddCons(scip, cons);
    SCIPreleaseCons(scip, &cons);

    SCIPsetIntParam(scip, "presolving/maxrounds", 0);

    // solve the model
    // NOTE no subtour elimination constraints are added!
    SCIPsolve(scip);

    int num_expected_cc_conss;
    int num_actual_cc_conss = cycle_cover_conshdlr->getNumConssAdded();
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