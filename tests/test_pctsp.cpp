
#include "pctsp/logger.hh"
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    PCTSPinitLogging(2);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
