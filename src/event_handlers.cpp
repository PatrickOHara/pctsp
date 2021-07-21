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
    // if (SCIPgetStage(scip) >= SCIP_STAGE_SOLVING) {
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
        double lower_bound = SCIPnodeGetLowerbound(node);
        double upper_bound = SCIPgetUpperbound(scip);
        unsigned int parent_id = 1;
        if (node_id > 1) {
            SCIP_NODE* parent = SCIPnodeGetParent(node);
            parent_id = SCIPnodeGetNumber(parent);
        }
        unsigned int num_sec_disjoint_tour = 0;
        unsigned int num_sec_maxflow_mincut = 0;
        unsigned int num_cost_cover_disjoint_paths = 0;
        unsigned int num_cost_cover_shortest_paths = 0;
        unsigned int num_cost_cover_steiner_tree = 0;
        NodeStats node_stats = {lower_bound, node_id, num_sec_disjoint_tour, num_sec_maxflow_mincut,
            num_cost_cover_disjoint_paths, num_cost_cover_shortest_paths, num_cost_cover_steiner_tree,
            parent_id, upper_bound};
            stats_vector[node_id-1] = node_stats;
    // }
    return SCIP_OKAY;
}
