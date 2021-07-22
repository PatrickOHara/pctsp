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
