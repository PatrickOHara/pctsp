#include <iostream>
#include "pctsp/event_handlers.hh"


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
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    std::vector<NodeStats> & stats_vector = *probdata->getNodeStats();
    // int num_nodes = SCIPgetNTotalNodes(scip);
    int stats_size = stats_vector.size();

    // get current node and update info about current node
    SCIP_NODE* node = SCIPgetCurrentNode(scip);
    unsigned int node_id = SCIPnodeGetNumber(node);
    if (stats_size < node_id) {
        // resize stats array
        stats_vector.resize(node_id);
    }
    NodeStats node_stats = stats_vector.at(node_id-1);
    // if id is zero then stats has not been initialised for this node
    if (node_stats.node_id == 0) {
        node_stats.node_id = node_id;
        node_stats.lower_bound = SCIPnodeGetLowerbound(node);
        node_stats.upper_bound = SCIPgetUpperbound(scip);
        node_stats.parent_id = 1;
        if (node_id > 1) {
            SCIP_NODE* parent = SCIPnodeGetParent(node);
            node_stats.parent_id = SCIPnodeGetNumber(parent);
        }
        stats_vector[node_id-1] = node_stats;
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
