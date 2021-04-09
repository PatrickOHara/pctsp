#include "pctsp/knapsack.hh"
#include <gtest/gtest.h>

TEST(TestKnapsack, testKnapsack) {
    std::vector<int> costs = {1, 2, 3, 4};
    std::vector<int> weights = {20, 15, 35, 15};
    int capacity = 50;
    SCIP_RETCODE code = knapsack(costs, weights, capacity);
    EXPECT_EQ(code, SCIP_OKAY);
}
