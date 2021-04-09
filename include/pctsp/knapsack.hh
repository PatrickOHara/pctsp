#ifndef __PCTSP_KNAPSACK__
#define __PCTSP_KNAPSACK__

/**
 * Algorithms for the knapsack problem
 */

#include <map>
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>
#include <vector>

using namespace scip;
using namespace std;

/** Solve a knapsack problem */
SCIP_RETCODE knapsack(std::vector<int> &costs, std::vector<int> &weights,
                      int capacity);
#endif