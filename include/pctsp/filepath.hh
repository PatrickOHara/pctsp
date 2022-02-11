#ifndef __PCTSP_FILEPATH__
#define __PCTSP_FILEPATH__

#include <filesystem>
#include <string>

const std::string BOUNDS_CSV_FILENAME = "bounds.csv";
const std::string METRICS_CSV_FILENAME = "metrics.csv";
const std::string SCIP_LOGS_FILENAME = "scip_logs.txt";
const std::string SUMMARY_STATS_FILENAME = "summary_stats.yml";

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