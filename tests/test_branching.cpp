#include "pctsp/branching.hh"
#include "fixtures.hh"
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>


TEST(TestBranching, testSetStrongBranchingStrategy) {
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);
    setStrongBranchingStrategy(scip);
    SCIP_BRANCHRULE* strong = findStrongBranchingRule(scip);
    SCIP_BRANCHRULE* relpscost = findRelPsCostBranchingRule(scip);
    EXPECT_EQ(SCIPbranchruleGetPriority(strong), 4000);
    EXPECT_EQ(SCIPbranchruleGetPriority(relpscost), 3000);
    SCIPfree(&scip);
}

TEST(TestBranching, testSetBranchingStrategy) {
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);
    int max_depth = 1;
    setBranchingStrategy(scip, BranchingStrategy::STRONG_AT_TREE_TOP, max_depth);
    SCIP_BRANCHRULE* strong = findStrongBranchingRule(scip);
    SCIP_BRANCHRULE* relpscost = findRelPsCostBranchingRule(scip);
    EXPECT_EQ(SCIPbranchruleGetPriority(strong), 4000);
    EXPECT_EQ(SCIPbranchruleGetMaxdepth(strong), max_depth);
    EXPECT_EQ(SCIPbranchruleGetPriority(relpscost), 3000);
    SCIPfree(&scip);
}
