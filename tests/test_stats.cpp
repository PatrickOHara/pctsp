
#include "pctsp/stats.hh"
#include <gtest/gtest.h>


TEST(TestStats, testWriteNodeStatsToCSV) {
    NodeStats node = { 5.2, 0, 0, 1, 0, 0, 2, 0, 7.0 };
    std::vector<NodeStats> stats = { node };
    std::string file_path = ".logs/test_node_stats.csv";
    writeNodeStatsToCSV(stats, file_path);
}