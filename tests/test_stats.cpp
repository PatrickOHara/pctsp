
#include "pctsp/stats.hh"
#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>
#include <boost/filesystem.hpp>

TEST(TestStats, testWriteNodeStatsToCSV) {
    NodeStats node = { 5.2, 0, 0, 1, 0, 0, 2, 0, 7.0 };
    std::vector<NodeStats> stats = { node };
    std::string file_path = ".logs/test_node_stats.csv";
    writeNodeStatsToCSV(stats, file_path);
}

TEST(TestYamlCpp, testYamlCppIsFound) {
    YAML::Node config = YAML::LoadFile("mkdocs.yml");

    if (config["site_name"]) {
        EXPECT_EQ(config["site_name"].as<std::string>(), "pctsp");
    }
}

TEST(TestYamlCpp, testWriteSummaryStatsToYaml) {
    SummaryStats summary = {SCIP_Status::SCIP_STATUS_OPTIMAL, 1.0, 2.0, 2, 0, 5, 0, 8};
    std::string filename = ".logs/summary.yaml";
    writeSummaryStatsToYaml(summary, filename);
    EXPECT_TRUE(boost::filesystem::exists(filename));
}