#include <gtest/gtest.h>
#include <scip/scip.h>

TEST(TestRandom, testSCIPgetRandomReal) {
    unsigned int seed = 100;
    unsigned int n = 10;
    std::vector<unsigned int> expected (n);
    SCIPrandomGetInt()
    for (int i = 0; i < n; i++) {
        expected[i] = SCIPgetRandomInt()
    }
    SCIPgetRandomReal
}