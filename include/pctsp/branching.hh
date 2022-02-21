#ifndef __PCTSP_BRANCHING__
#define __PCTSP_BRANCHING__

#include <string>
#include <scip/scip.h>

const unsigned int PCTSP_DEFAULT_SEED = 1;

struct BranchingStrategy {
    static const unsigned int RELPSCOST;
    static const unsigned int STRONG;
    static const unsigned int STRONG_AT_TREE_TOP;
};

struct BRANCHING_RULE_NAMES {
    static const std::string LEAST_INFEASIBLE;
    static const std::string MOST_INFEASIBLE;
    static const std::string FULL_STRONG;
    static const std::string RELPSCOST;
};

void includeBranchRules(SCIP* scip);

SCIP_BRANCHRULE* findStrongBranchingRule(SCIP* scip);

SCIP_BRANCHRULE* findRelPsCostBranchingRule(SCIP* scip);

void setStrongBranchingStrategy(SCIP* scip);

void setStrongAtTreeTopBranchingStrategy(SCIP* scip, int strong_branching_max_depth);

void setBranchingStrategy(SCIP* scip, unsigned int strategy, int max_depth);

void setBranchingStrategy(SCIP* scip, unsigned int strategy);

void setBranchingRandomSeeds(SCIP* scip, unsigned int seed);

void setBranchingRandomSeeds(SCIP* scip);

#endif