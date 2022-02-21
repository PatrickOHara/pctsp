#include <iostream>
#include "pctsp/branching.hh"
#include <scip/scipdefplugins.h>

const std::string BRANCHING_RULE_NAMES::LEAST_INFEASIBLE = "leastinf";
const std::string BRANCHING_RULE_NAMES::MOST_INFEASIBLE = "mostinf";
const std::string BRANCHING_RULE_NAMES::FULL_STRONG = "fullstrong";
const std::string BRANCHING_RULE_NAMES::RELPSCOST = "relpscost";

const unsigned int BranchingStrategy::RELPSCOST = 0;
const unsigned int BranchingStrategy::STRONG = 1;
const unsigned int BranchingStrategy::STRONG_AT_TREE_TOP = 2;

void includeBranchRules(SCIP* scip) {
    SCIPincludeBranchruleMostinf(scip);
    SCIPincludeBranchruleFullstrong(scip);
}

SCIP_BRANCHRULE* findStrongBranchingRule(SCIP* scip) {
    SCIP_BRANCHRULE* branchrule = SCIPfindBranchrule(scip, BRANCHING_RULE_NAMES::FULL_STRONG.c_str());
    return branchrule;
}

SCIP_BRANCHRULE* findRelPsCostBranchingRule(SCIP* scip) {
    SCIP_BRANCHRULE* branchrule = SCIPfindBranchrule(scip, BRANCHING_RULE_NAMES::RELPSCOST.c_str());
    return branchrule;
}

/**
 * @brief Set seed of every branching rule to predict randomness
 */
void setBranchingRandomSeeds(SCIP* scip, unsigned int seed) {
    // SCIPsetIntParam(scip, "branching/random/seed", seed);
    // SCIPsetIntParam(scip, "branching/relpscost/startrandseed", seed);
    // SCIPsetIntParam(scip, "heuristics/alns/seed", seed);
    // SCIPsetIntParam(scip, "randomization/lpseed", seed);
    // SCIPsetIntParam(scip, "randomization/permutationseed", seed);
    // SCIPsetIntParam(scip, "separating/zerohalf/initseed", seed);

    // SCIPsetBoolParam(scip, "concurrent/changeseeds", false);
    // SCIPsetIntParam(scip, "concurrent/initseed", seed);

    // SCIP_BRANCHRULE* relpscost = findRelPsCostBranchingRule(scip);
    // auto branchruledata = SCIPbranchruleGetData(relpscost);
    // SCIPsetRandomSeed(scip, branchruledata->randnumgen, seed);
}

/**
 * @brief Set the seed of all branching rules with randomness to be PCTSP_DEFAULT_SEED 
 */
void setBranchingRandomSeeds(SCIP* scip) {
    setBranchingRandomSeeds(scip, PCTSP_DEFAULT_SEED);
}

void setStrongBranchingStrategy(SCIP* scip) {
    auto n = SCIPgetNBranchrules(scip);
    SCIP_BRANCHRULE** rules = SCIPgetBranchrules(scip);
    for (int i = 0; i < n; i++) {
        SCIP_BRANCHRULE* rule = rules[i];
        std::string name = SCIPbranchruleGetName(rule);
        std::cout << "Branching rule " << name << " included." << std::endl;
        if (name == BRANCHING_RULE_NAMES::FULL_STRONG) {
            // set strong branching to be highest priority
            SCIPsetBranchrulePriority(scip, rule, 4000);
        }
        else if (name == BRANCHING_RULE_NAMES::RELPSCOST) {
            // set relative psuedo cost to be second priority
            // TODO change back to positive 3000
            SCIPsetBranchrulePriority(scip, rule, 3000);
        }
        else if (name == BRANCHING_RULE_NAMES::MOST_INFEASIBLE) {
            // TODO set this to be something sensible
            SCIPsetBranchrulePriority(scip, rule, 1000);
        }
        else {
            // set the priority of other rules that used to be high priority to be executed third
            auto priority = SCIPbranchruleGetPriority(rule);
            if (priority >= 3000) {
                SCIPsetBranchrulePriority(scip, rule, 2500);
            }
        }
    }
}

void setStrongAtTreeTopBranchingStrategy(SCIP* scip, int strong_branching_max_depth) {
    // use the same priority for each branching rule
    setStrongBranchingStrategy(scip);

    // only use strong branching for B&B nodes at depth <= max depth
    SCIP_BRANCHRULE* strong = findStrongBranchingRule(scip);
    SCIPsetBranchruleMaxdepth(scip, strong, strong_branching_max_depth);
}

void setBranchingStrategy(SCIP* scip, unsigned int strategy, int max_depth) {
    switch (strategy) {
        case BranchingStrategy::RELPSCOST: {
            break;
        }
        case BranchingStrategy::STRONG: {
            setStrongBranchingStrategy(scip);
            break;
        }
        case BranchingStrategy::STRONG_AT_TREE_TOP: {
            setStrongAtTreeTopBranchingStrategy(scip, max_depth);
            break;
        }
        default: break; // use existing strategy
    }
}

void setBranchingStrategy(SCIP* scip, unsigned int strategy) {
    setBranchingStrategy(scip, strategy, -1);
}