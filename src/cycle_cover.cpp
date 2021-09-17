#include "pctsp/cycle_cover.hh"

SCIP_RETCODE addCycleCover(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    PCTSPgraph& graph,
    std::vector<PCTSPvertex>& vertices_in_cover,
    PCTSPedgeVariableMap& edge_variable_map,
    SCIP_SOL* sol,
    SCIP_RESULT* result,
    std::string& name
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
        result,
        name
    );
}