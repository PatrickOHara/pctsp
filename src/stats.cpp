#include "pctsp/stats.hh"
#include <yaml-cpp/yaml.h>

void writeSummaryStatsToYaml(SummaryStats& summary, std::string& filename) {
    YAML::Node node;
    node["status"] = enum_as_integer(summary.status);
    node["lower_bound"] = summary.lower_bound;
    node["upper_bound"] = summary.upper_bound;
    node["num_cost_cover_disjoint_paths"] = summary.num_cost_cover_disjoint_paths;
    node["num_cost_cover_shortest_paths"] = summary.num_cost_cover_shortest_paths;
    node["num_cycle_cover"] = summary.num_cycle_cover;
    node["num_nodes"] = summary.num_nodes;
    node["num_sec_disjoint_tour"] = summary.num_sec_disjoint_tour;
    node["num_sec_maxflow_mincut"] = summary.num_sec_maxflow_mincut;
    std::ofstream fout(filename);
    fout << node;
}

SummaryStats getSummaryStatsFromSCIP(
    SCIP* scip,
    unsigned int num_cost_cover_disjoint_paths,
    unsigned int num_cost_cover_shortest_paths,
    unsigned int num_cycle_cover,
    unsigned int num_sec_disjoint_tour,
    unsigned int num_sec_maxflow_mincut
) {

    SummaryStats summary = {
        SCIPgetStatus(scip),
        SCIPgetLowerbound(scip),
        SCIPgetUpperbound(scip),
        num_cost_cover_disjoint_paths,
        num_cost_cover_shortest_paths,
        num_cycle_cover,
        SCIPgetNNodes(scip),
        num_sec_disjoint_tour,
        num_sec_maxflow_mincut
    };
    return summary;
}

unsigned int numDisjointTourSECs(std::vector<NodeStats>& node_stats) {
    unsigned int total = 0;
    for (NodeStats& stat : node_stats) {
        total += stat.num_sec_disjoint_tour;
    }
    return total;
}

unsigned int numMaxflowMincutSECs(std::vector<NodeStats>& node_stats) {
    unsigned int total = 0;
    for (NodeStats& stat : node_stats) {
        total += stat.num_sec_maxflow_mincut;
    }
    return total;
}

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
    auto first = NODE_STATS_COL_NAMES.begin();
    auto last =  NODE_STATS_COL_NAMES.end();
    writeRowCSV(csv_file, first, last);
}

void writeBoundsToCSV(std::vector<Bounds>& bounds_vector, std::string& file_path) {
    std::ofstream csv_file(file_path);
    writeBoundsColumnsNames(csv_file);
    for (Bounds& bounds : bounds_vector) {
        writeBoundsRow(bounds, csv_file);
    }
}

void writeBoundsColumnsNames(std::ofstream& csv_file) {
    auto first = BOUNDS_COLUMN_NAMES.begin();
    auto last = BOUNDS_COLUMN_NAMES.end();
    writeRowCSV(csv_file, first, last);
}

void writeBoundsRow(Bounds& bounds, std::ofstream& csv_file) {
    csv_file << timePointToString(bounds.start_timestamp) << ",";
    csv_file << timePointToString(bounds.end_timestamp) << ",";
    csv_file << bounds.lower_bound << ",";
    csv_file << bounds.upper_bound << ",";
    csv_file << bounds.node_id << "\n";
}
