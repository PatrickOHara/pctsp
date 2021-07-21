#include "pctsp/stats.hh"

void writeNodeStatsToCSV(std::vector<NodeStats>& node_stats, std::string& file_path) {
    std::ofstream csv_file(file_path);
    writeNodeStatsColumnNames(csv_file);
    for (NodeStats& stat : node_stats) {
        writeNodeStatsRow(stat, csv_file);
    }
}

void writeNodeStatsRow(NodeStats& node_stats, std::ofstream& csv_file) {
    csv_file << node_stats.lower_bound << ",";
    csv_file << node_stats.node_id << ",";
    csv_file << node_stats.num_sec_disjoint_tour << ",";
    csv_file << node_stats.num_sec_maxflow_mincut << ",";
    csv_file << node_stats.num_cost_cover_disjoint_paths << ",";
    csv_file << node_stats.num_cost_cover_shortest_paths << ",";
    csv_file << node_stats.num_cost_cover_steiner_tree << ",";
    csv_file << node_stats.parent_id << ",";
    csv_file << node_stats.upper_bound << "\n";     // new line at end of row
}

void writeNodeStatsColumnNames(std::ofstream& csv_file) {

    for (int i = 0; i < NODE_STATS_COL_NAMES.size(); i++) {
        csv_file << NODE_STATS_COL_NAMES.at(i);
        if (i < NODE_STATS_COL_NAMES.size() - 1) csv_file << ",";
    }
    csv_file << "\n";
}
