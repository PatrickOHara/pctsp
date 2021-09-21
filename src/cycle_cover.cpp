#include "pctsp/cycle_cover.hh"

SCIP_RETCODE addCycleCover(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    PCTSPgraph& graph,
    std::vector<PCTSPvertex>& vertices_in_cover,
    PCTSPedgeVariableMap& edge_variable_map,
    SCIP_SOL* sol,
    SCIP_RESULT* result
) {
    auto first = vertices_in_cover.begin();
    auto last = vertices_in_cover.end();
    return addCycleCover(
        scip,
        conshdlr,
        graph,
        first,
        last,
        edge_variable_map,
        sol,
        result
    );
}

SCIP_RETCODE createCycleCoverCons(
    SCIP* scip,
    SCIP_CONS** cons,
    const std::string& name,
    SCIP_Bool initial,
    SCIP_Bool separate,
    SCIP_Bool enforce,
    SCIP_Bool check,
    SCIP_Bool propagate,
    SCIP_Bool local,
    SCIP_Bool modifiable,
    SCIP_Bool dynamic,
    SCIP_Bool removable
) {
    SCIP_CONSHDLR* conshdlr;
    SCIP_CONSDATA* consdata;

    /* find the subtour constraint handler */
    conshdlr = SCIPfindConshdlr(scip, CYCLE_COVER_NAME.c_str());
    if (conshdlr == NULL)
    {
        std::string error_message = CYCLE_COVER_NAME + ": constraint handler not found.";
        auto cstr_message = error_message.c_str();
        SCIPmessagePrintError("%s", cstr_message);
        return SCIP_PLUGINNOTFOUND;
    }
    return SCIPcreateCons(scip, cons, name.c_str(), conshdlr, consdata, initial, separate, enforce, check, propagate,
        local, modifiable, dynamic, removable, FALSE);
}

SCIP_RETCODE createBasicCycleCoverCons(SCIP* scip, SCIP_CONS** cons) {
    return createBasicCycleCoverCons(scip, cons, CYCLE_COVER_CONS_PREFIX);
}

SCIP_RETCODE createBasicCycleCoverCons(SCIP* scip, SCIP_CONS** cons, const std::string& name) {
    return createCycleCoverCons(
        scip,
        cons,
        name,
        FALSE,
        TRUE,
        TRUE,
        TRUE,
        TRUE,
        FALSE,
        FALSE,
        FALSE,
        TRUE
    );
}

bool isCycleCoverViolated(SCIP* scip, SCIP_SOL* sol, ProbDataPCTSP* probdata) {
    auto& input_graph = *(probdata->getInputGraph());
    auto& edge_variable_map = *(probdata->getEdgeVariableMap());
    auto& root_vertex = *(probdata->getRootVertex());
    auto& quota = *(probdata->getQuota());
    auto prize_map = boost::get(vertex_distance, input_graph);
    return isCycleCoverViolated(scip, input_graph, prize_map, quota, root_vertex, edge_variable_map, sol);
}

SCIP_RETCODE separateCycleCover(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_SOL* sol, SCIP_RESULT* result) {
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    auto& input_graph = *(probdata->getInputGraph());
    auto& edge_variable_map = *(probdata->getEdgeVariableMap());
    auto& root_vertex = *(probdata->getRootVertex());
    auto& quota = *(probdata->getQuota());
    auto prize_map = boost::get(vertex_distance, input_graph);
    return separateCycleCover(scip, conshdlr, sol, result, input_graph, prize_map, quota, root_vertex, edge_variable_map);
}

int CycleCoverConshdlr::getNumConssAdded() {
    return _num_conss_added;
}

SCIP_DECL_CONSCHECK(CycleCoverConshdlr::scip_check) {
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    if (isCycleCoverViolated(scip, sol, probdata)) {
        *result = SCIP_INFEASIBLE;
    }
    else {
        *result = SCIP_FEASIBLE;
    }
    return SCIP_OKAY;
}

SCIP_DECL_CONSENFOPS(CycleCoverConshdlr::scip_enfops) {
    return SCIP_OKAY;
}
SCIP_DECL_CONSENFOLP(CycleCoverConshdlr::scip_enfolp) {
    return SCIP_OKAY;
}
SCIP_DECL_CONSTRANS(CycleCoverConshdlr::scip_trans) {
    return SCIP_OKAY;
}
SCIP_DECL_CONSLOCK(CycleCoverConshdlr::scip_lock) {
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPALP(CycleCoverConshdlr::scip_sepalp) {
    SCIP_SOL* sol = NULL;
    return separateCycleCover(scip, conshdlr, sol, result);
}
