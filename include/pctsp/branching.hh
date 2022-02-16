#ifndef __PCTSP_BRANCHING__
#define __PCTSP_BRANCHING__

#include <string>
#include <scip/scip.h>

struct BranchingStrategy {
    static const unsigned int RELPSCOST;
    static const unsigned int STRONG;
    static const unsigned int STRONG_AT_TREE_TOP;
};

struct BRANCHING_RULE_NAMES {
    static const std::string FULL_STRONG;
    static const std::string RELPSCOST;
};

SCIP_BRANCHRULE* findStrongBranchingRule(SCIP* scip);

SCIP_BRANCHRULE* findRelPsCostBranchingRule(SCIP* scip);

void setStrongBranchingStrategy(SCIP* scip);

void setStrongAtTreeTopBranchingStrategy(SCIP* scip, unsigned int strong_branching_max_depth);

void setBranchingStrategy(SCIP* scip, unsigned int strategy, unsigned int max_depth);

void setBranchingStrategy(SCIP* scip, unsigned int strategy);

#endif