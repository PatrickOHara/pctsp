#include <iostream>
#include "pctsp/event_handlers.hh"


unsigned int currentNodeId(SCIP* scip) {
    SCIP_NODE* node = SCIPgetCurrentNode(scip);
    unsigned int node_id = SCIPnodeGetNumber(node);
    return node_id;
}

NodeStats newStatsForNode(SCIP* scip, SCIP_NODE* node) {
   unsigned int node_id = SCIPnodeGetNumber(node);
   unsigned int parent_id = 1;
   if (node_id > 1) parent_id = SCIPnodeGetNumber(SCIPnodeGetParent(node));
   NodeStats stats = {
      SCIPnodeGetLowerbound(node),
      node_id,
      0,
      0,
      0,
      0,
      0,
      parent_id,
      SCIPgetUpperbound(scip)
   };
   return stats;
}

NodeStats NodeEventhdlr::getNodeStats(SCIP* scip) {
    SCIP_NODE* node = SCIPgetCurrentNode(scip);
    return getNodeStats(node);
}

NodeStats NodeEventhdlr::getNodeStats(SCIP_NODE* node) {
    unsigned int node_id = SCIPnodeGetNumber(node);
    NodeStats stats = node_stats_[node_id - 1];
    return stats;
}

std::vector<NodeStats> NodeEventhdlr::getNodeStatsVector() {
    return node_stats_;
}

void NodeEventhdlr::addCurrentNode(SCIP* scip) {
    SCIP_NODE* node = SCIPgetCurrentNode(scip);
    NodeStats stats = newStatsForNode(scip, node);
    auto node_id = SCIPnodeGetNumber(node);
    auto node_index = node_id - 1;
    if (node_stats_.size() < node_id) node_stats_.resize(node_id);
    node_stats_[node_index] = stats;
}

void NodeEventhdlr::incrementNumSecDisjointTour(SCIP* scip, unsigned int n_cuts) {
    auto node_index = currentNodeId(scip) - 1;
    node_stats_[node_index].num_sec_disjoint_tour += n_cuts;
}

void NodeEventhdlr::incrementNumSecMaxflowMincut(SCIP* scip, unsigned int n_cuts) {
    auto node_index = currentNodeId(scip) - 1;
    node_stats_[node_index].num_sec_maxflow_mincut += n_cuts;
}


SCIP_DECL_EVENTFREE(NodeEventhdlr::scip_free) {
   return SCIP_OKAY;
}

SCIP_DECL_EVENTINIT(NodeEventhdlr::scip_init) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTEXIT(NodeEventhdlr::scip_exit) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTINITSOL(NodeEventhdlr::scip_initsol) {
    SCIP_CALL( SCIPcatchEvent( scip, SCIP_EVENTTYPE_NODEEVENT, eventhdlr, NULL, NULL) );
    return SCIP_OKAY;
}

SCIP_DECL_EVENTEXITSOL(NodeEventhdlr::scip_exitsol) {
    SCIP_CALL( SCIPdropEvent( scip, SCIP_EVENTTYPE_NODEEVENT, eventhdlr, NULL, -1) );
    return SCIP_OKAY;
}

SCIP_DECL_EVENTDELETE(NodeEventhdlr::scip_delete) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTEXEC(NodeEventhdlr::scip_exec) {
    SCIP_NODE* node = SCIPgetCurrentNode(scip);
    auto node_id = SCIPnodeGetNumber(node);
    if (node_id >= node_stats_.size()) {
        addCurrentNode(scip);
    }
    return SCIP_OKAY;
}

// bound handler 

std::string timePointToString(TimePointUTC& time_stamp) {
    using namespace std::chrono;
    auto ms = duration_cast<SubSeconds>(time_stamp.time_since_epoch()) % 1000;
    time_t tt = std::chrono::system_clock::to_time_t(time_stamp);
    tm utc_tm = *gmtime(&tt);
    std::string time_zone_string = "Z";
    char mbstr[100];
    std::strftime(mbstr, sizeof(mbstr), "%FT%T", std::localtime(&tt));
    std::string all = std::string(mbstr) + "." + std::to_string(ms.count()) + time_zone_string;
    return all;
}


TimePointUTC BoundsEventHandler::getLastTimestamp() {
    return _last_timestamp;
}

std::vector<Bounds> BoundsEventHandler::getBoundsVector() {
    return _bounds_vector;
}

SCIP_DECL_EVENTFREE(BoundsEventHandler::scip_free) {
   return SCIP_OKAY;
}

SCIP_DECL_EVENTINIT(BoundsEventHandler::scip_init) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTEXIT(BoundsEventHandler::scip_exit) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTINITSOL(BoundsEventHandler::scip_initsol) {
    SCIP_CALL( SCIPcatchEvent( scip, SCIP_EVENTTYPE_LPSOLVED, eventhdlr, NULL, NULL) );
    return SCIP_OKAY;
}

SCIP_DECL_EVENTEXITSOL(BoundsEventHandler::scip_exitsol) {
    SCIP_CALL( SCIPdropEvent( scip, SCIP_EVENTTYPE_LPSOLVED, eventhdlr, NULL, -1) );
    return SCIP_OKAY;
}

SCIP_DECL_EVENTDELETE(BoundsEventHandler::scip_delete) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTEXEC(BoundsEventHandler::scip_exec) {
    using namespace std::chrono;
    TimePointUTC start;
    start = getLastTimestamp();
    TimePointUTC end = time_point_cast<SubSeconds>(system_clock::now());
    double lower_bound = SCIPgetLowerbound(scip);
    double upper_bound = SCIPgetUpperbound(scip);
    SCIP_NODE* node = SCIPgetCurrentNode(scip);
    unsigned int node_id = SCIPnodeGetNumber(node);
    Bounds bounds = {start, end, lower_bound, upper_bound, node_id};
    _bounds_vector.push_back(bounds);
    _last_timestamp = time_point_cast<SubSeconds>(system_clock::now());
    return SCIP_OKAY;
}
