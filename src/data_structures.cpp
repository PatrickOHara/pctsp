#include "pctsp/data_structures.hh"

PCTSPgraph* ProbDataPCTSP::getInputGraph() {
    return graph_;
}

int* ProbDataPCTSP::getQuota() {
    return quota_;
}

PCTSPvertex* ProbDataPCTSP::getRootVertex() {
    return root_vertex_;
}

PCTSPedgeVariableMap* ProbDataPCTSP::getEdgeVariableMap() {
    return edge_variable_map_;
}

std::vector<NodeStats>* ProbDataPCTSP::getNodeStats() {
    return node_stats_;
}