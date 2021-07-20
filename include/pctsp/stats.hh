
#ifndef __PCTSP_STATS__
#define __PCTSP_STATS__

#include <fstream>
#include <string>
#include <vector>


const std::vector<std::string> NODE_STATS_COL_NAMES = {
    "lower_bound",
    "node_id",
    "num_sec_disjoint_tour",
    "num_sec_maxflow_mincut",
    "num_cost_cover_disjoint_paths",
    "num_cost_cover_shortest_paths",
    "num_cost_cover_steiner_tree",
    "parent_id",
    "upper_bound",
};

/**
 * @brief Useful statistics to save for each node of the B&B tree
 */
struct NodeStats {
    double lower_bound;     // LP lower bound
    unsigned int node_id;   // Id of node
    unsigned int num_sec_disjoint_tour;         // number of SECs added with disjoint tour separation
    unsigned int num_sec_maxflow_mincut;        // number of SECs added with max flow
    unsigned int num_cost_cover_disjoint_paths; // num CC disjoint tour added
    unsigned int num_cost_cover_shortest_paths; // num CC shortest paths added
    unsigned int num_cost_cover_steiner_tree;   // num CC steiner tree added
    unsigned int parent_id;                     // ID of parent node
    double upper_bound;                         // lower bound of LP
};

void writeNodeStatsToCSV(std::vector<NodeStats>& node_stats, std::string& file_path);

void writeNodeStatsColumnNames(std::ofstream& csv_file);

void writeNodeStatsRow(NodeStats& node_stats, std::ofstream& csv_file);


#endif