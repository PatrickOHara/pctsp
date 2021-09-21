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

// SCIP_DECL_CONSENFOPS(CycleCoverConshdlr::scip_enfops);
// SCIP_DECL_CONSENFOLP(CycleCoverConshdlr::scip_enfolp);
// SCIP_DECL_CONSLOCK(CycleCoverConshdlr::scip_lock);

SCIP_DECL_CONSSEPALP(CycleCoverConshdlr::scip_sepalp) {
    SCIP_SOL* sol = NULL;
    return separateCycleCover(scip, conshdlr, sol, result);
}
