#include "pctsp/branching.hh"
#include "fixtures.hh"
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

typedef GraphFixture BranchingFixture;

TEST(TestBranching, testBranchRule) {
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);
    std::string branch_rule_name = "strong";
    SCIP_BRANCHRULE* branchrule = SCIPfindBranchrule(scip, branch_rule_name.c_str());
    auto n = SCIPgetNBranchrules(scip);
    EXPECT_EQ(n, 14);
    SCIP_BRANCHRULE** rules = SCIPgetBranchrules(scip);
    for (int i = 0; i < n; i++) {
        SCIP_BRANCHRULE* rule = rules[i];
        std::cout << SCIPbranchruleGetName(rule) << ": priority = " << SCIPbranchruleGetPriority(rule)
        << ". max depth = " << SCIPbranchruleGetMaxdepth(rule) << std::endl;
    }
}

TEST(TestBranching, testSetStrongBranchingStrategy) {
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);
    setStrongBranchingStrategy(scip);
    SCIP_BRANCHRULE* strong = findStrongBranchingRule(scip);
    SCIP_BRANCHRULE* relpscost = findRelPsCostBranchingRule(scip);
    EXPECT_EQ(SCIPbranchruleGetPriority(strong), 4000);
    EXPECT_EQ(SCIPbranchruleGetPriority(relpscost), 3000);
}

TEST(TestBranching, testSetBranchingStrategy) {
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);
    unsigned int max_depth = 1;
    setBranchingStrategy(scip, BranchingStrategy::STRONG_AT_TREE_TOP, max_depth);
    SCIP_BRANCHRULE* strong = findStrongBranchingRule(scip);
    SCIP_BRANCHRULE* relpscost = findRelPsCostBranchingRule(scip);
    EXPECT_EQ(SCIPbranchruleGetPriority(strong), 4000);
    EXPECT_EQ(SCIPbranchruleGetMaxdepth(strong), max_depth);
    EXPECT_EQ(SCIPbranchruleGetPriority(relpscost), 3000);
}

TEST_P(BranchingFixture, testBranchRuleCodes) {


}

INSTANTIATE_TEST_SUITE_P(
    TestBranching,
    BranchingFixture,
    ::testing::Values(GraphType::GRID8, GraphType::SUURBALLE, GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::COMPLETE50)
);
