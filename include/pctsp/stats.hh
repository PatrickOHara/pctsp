
#ifndef __PCTSP_STATS__
#define __PCTSP_STATS__

#include <chrono>
#include <fstream>
#include <string>
#include <vector>
#include <objscip/objscip.h>
#include "pctsp/filepath.hh"

struct SummaryStats {
    SCIP_Status status;
    double lower_bound;
    double upper_bound;
    unsigned int num_cost_cover_disjoint_paths;
    unsigned int num_cost_cover_shortest_paths;
    unsigned int nconss_presolve_disjoint_paths;
    unsigned int nconss_presolve_shortest_paths;
    unsigned int num_cycle_cover;
    long long num_nodes;
    unsigned int num_sec_disjoint_tour;         // number of SECs added with disjoint tour separation
    unsigned int num_sec_maxflow_mincut;        // number of SECs added with max flow
};

SummaryStats readSummaryStatsFromYaml(std::filesystem::path& filename);

void writeSummaryStatsToYaml(SummaryStats& summary, std::filesystem::path& filename);

SummaryStats getSummaryStatsFromSCIP(
    SCIP* scip,
    unsigned int num_cost_cover_disjoint_paths,
    unsigned int num_cost_cover_shortest_paths,
    unsigned int num_cycle_cover,
    unsigned int num_sec_disjoint_tour,
    unsigned int num_sec_maxflow_mincut
);

template <typename Enumeration>
auto enum_as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

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

unsigned int numDisjointTourSECs(std::vector<NodeStats>& node_stats);

unsigned int numMaxflowMincutSECs(std::vector<NodeStats>& node_stats);

// void writeNodeStatsToCSV(std::vector<NodeStats>& node_stats, std::string& file_path);

void writeNodeStatsToCSV(std::vector<NodeStats>& node_stats, std::filesystem::path& file_path);

void writeNodeStatsColumnNames(std::ofstream& csv_file);

void writeNodeStatsRow(NodeStats& node_stats, std::ofstream& csv_file);

typedef typename std::chrono::milliseconds SubSeconds;
typedef typename std::chrono::time_point<std::chrono::system_clock, SubSeconds> TimePointUTC;

std::string timePointToString(TimePointUTC& time_stamp);

const std::vector<std::string> BOUNDS_COLUMN_NAMES = {
    "start_timestamp",
    "end_timestamp",
    "lower_bound",
    "upper_bound",
    "node_id"
};

struct Bounds {
   TimePointUTC start_timestamp;
   TimePointUTC end_timestamp;
   double lower_bound;
   double upper_bound;
   unsigned int node_id;
};

void writeBoundsToCSV(std::vector<Bounds>& bounds_vector, std::filesystem::path& file_path);

void writeBoundsColumnsNames(std::ofstream& csv_file);

void writeBoundsRow(Bounds& bounds, std::ofstream& csv_file);

#endif