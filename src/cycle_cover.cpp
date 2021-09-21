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

SCIP_DECL_CONSCHECK(CycleCoverConshdlr::scip_check) {

}
// SCIP_DECL_CONSENFOPS(CycleCoverConshdlr::scip_enfops);
// SCIP_DECL_CONSENFOLP(CycleCoverConshdlr::scip_enfolp);
// SCIP_DECL_CONSLOCK(CycleCoverConshdlr::scip_lock);

SCIP_DECL_CONSSEPALP(CycleCoverConshdlr::scip_sepalp) {

}