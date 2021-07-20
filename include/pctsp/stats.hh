
#ifndef __PCTSP_GRAPH__
#define __PCTSP_GRAPH__

#include <vector>


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

void writeNodeStatsToCSV(std::vector<NodeStats>& node_stats);

#endif