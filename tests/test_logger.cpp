

#include "pctsp/logger.hh"
#include <gtest/gtest.h>



TEST(TestLogger, testBasicLogger) {
    int py_warning_level = 30;
    int level = getBoostLevelFromPyLevel(py_warning_level);
    EXPECT_EQ(level, (int)logging::trivial::warning);
    PCTSPinitLogging(level);

    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";
}

TEST(TestLogger, testLoggingSize) {
    int py_warning_level = 30;
    int level = getBoostLevelFromPyLevel(py_warning_level);
    PCTSPinitLogging(level);
    std::vector<int> myVector = {1,2,3,4};
    BOOST_LOG_TRIVIAL(warning) << "Size of myVector: " << std::to_string(myVector.size());
}