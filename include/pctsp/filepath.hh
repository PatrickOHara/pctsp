#ifndef __PCTSP_FILEPATH__
#define __PCTSP_FILEPATH__

#include <filesystem>
#include <string>

const std::string BOOST_LOGS_FILENAME = "boost_logs.txt";
const std::string SCIP_BOUNDS_CSV = "lower_upper_bounds.csv";
const std::string SCIP_LOGS_TXT = "scip_logs.txt";
const std::string SCIP_NODE_STATS_CSV = "scip_node_stats.csv";
const std::string PCTSP_SUMMARY_STATS_YAML = "pctsp_summary_stats.yaml";

template<typename StringIt>
void writeRowCSV(std::ofstream& csv_file, StringIt& first, StringIt& last) {
    auto num_items = std::distance(first, last);
    for (int i = 0; i < num_items; i++) {
        csv_file << *first++;
        if (i < num_items - 1) csv_file << ",";
    }
    csv_file << "\n";
}

#endif