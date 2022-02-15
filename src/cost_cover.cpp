#include "pctsp/cost_cover.hh"
#include "pctsp/data_structures.hh"
#include "pctsp/logger.hh"
#include "pctsp/sciputils.hh"

SCIP_RETCODE addCoverInequality(
    SCIP* scip,
    std::vector<SCIP_VAR*>& variables,
    SCIP_RESULT* result,
    SCIP_SOL* sol
) {
    // at least one variable must be zero
    // x(S) <= |x(S)| - 1 
    int nvars = variables.size();
    std::vector<double> var_coefs(nvars);
    BOOST_LOG_TRIVIAL(debug) << nvars << " variables added to cover inequality.";
    for (int i = 0; i < nvars; i ++) {
        var_coefs[i] = 1;
    }
    double lhs = -SCIPinfinity(scip);
    double rhs = nvars - 1;
    SCIP_VAR** vars = variables.data();
    double* coefs = var_coefs.data();

    SCIP_CONS* cons;
    std::string cons_name_str = COST_COVER_CONS_PREFIX + joinVariableNames(variables);
    const char* cons_name = cons_name_str.c_str();
    SCIP_CALL( SCIPcreateConsLinear(scip, &cons, cons_name, nvars, vars, coefs, lhs, rhs,
         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE) );
    SCIPaddCons(scip, cons);
    SCIPreleaseCons(scip, &cons);
    return SCIP_OKAY;
}

// Cost cover event handler for new solutions

unsigned int CostCoverEventHandler::getNumConssAdded() {
    return _num_conss_added;
}

std::vector<int> CostCoverEventHandler::getPathDistances() {
    return _path_distances;
}

SCIP_DECL_EVENTFREE(CostCoverEventHandler::scip_free) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTINIT(CostCoverEventHandler::scip_init) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTEXIT(CostCoverEventHandler::scip_exit) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTINITSOL(CostCoverEventHandler::scip_initsol) {
    SCIP_CALL( SCIPcatchEvent( scip, SCIP_EVENTTYPE_BESTSOLFOUND, eventhdlr, NULL, NULL) );
    return SCIP_OKAY;
}

SCIP_DECL_EVENTEXITSOL(CostCoverEventHandler::scip_exitsol) {
    SCIP_CALL( SCIPdropEvent( scip, SCIP_EVENTTYPE_BESTSOLFOUND, eventhdlr, NULL, -1) );
    return SCIP_OKAY;
}

SCIP_DECL_EVENTDELETE(CostCoverEventHandler::scip_delete) {
    return SCIP_OKAY;
}

SCIP_DECL_EVENTEXEC(CostCoverEventHandler::scip_exec) {
    double cost_upper_bound = SCIPgetUpperbound(scip);
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    SCIP_RESULT* result;
    PCTSPgraph& graph = * probdata->getInputGraph();
    PCTSPvertex& root_vertex = *probdata->getRootVertex();
    PCTSPedgeVariableMap& edge_variable_map = *probdata->getEdgeVariableMap();

    auto violated_vertices = separateCostCoverPaths(graph, _path_distances, cost_upper_bound);
    bool violation_found = violated_vertices.size() > 0;
    for (auto& vertex: violated_vertices) {
        _num_conss_added ++;
        std::vector<PCTSPvertex> cover_vertices = {root_vertex, vertex};
        addCoverInequalityFromVertices(scip, graph, cover_vertices, edge_variable_map, result, sol);
    }
    return SCIP_OKAY;
}

SCIP_RETCODE includeCostCoverEventHandler(
    SCIP* scip,
    const std::string& name,
    const std::string& description,
    std::vector<int>& path_distances
)
{
    CostCoverEventHandler* handler = new CostCoverEventHandler(scip, name, description, path_distances);
    SCIP_CALL(SCIPincludeObjEventhdlr(scip, handler, TRUE));
    return SCIP_OKAY;
}

SCIP_RETCODE includeShortestPathCostCover(SCIP* scip, std::vector<int>& path_distances) {
    return includeCostCoverEventHandler(
        scip,
        SHORTEST_PATH_COST_COVER_NAME,
        COST_COVER_DESCRIPTION,
        path_distances
    );
}

SCIP_RETCODE includeDisjointPathsCostCover(SCIP* scip, std::vector<int>& path_distances) {
    return includeCostCoverEventHandler(
        scip,
        DISJOINT_PATHS_COST_COVER_NAME,
        COST_COVER_DESCRIPTION,
        path_distances
    );
}

CostCoverEventHandler getDisjointPathsCostCoverEventHandler(SCIP* scip) {
    CostCoverEventHandler* cc_hdlr = dynamic_cast<CostCoverEventHandler*>(
        SCIPfindObjEventhdlr(scip, DISJOINT_PATHS_COST_COVER_NAME.c_str())
    );
    return *cc_hdlr;
}

CostCoverEventHandler getShortestPathCostCoverEventHandler(SCIP* scip) {
    CostCoverEventHandler* cc_hdlr = dynamic_cast<CostCoverEventHandler*>(
        SCIPfindObjEventhdlr(scip, SHORTEST_PATH_COST_COVER_NAME.c_str())
    );
    return *cc_hdlr;
}

unsigned int getNShortestPathCostCoverCutsAdded(SCIP* scip) {
    CostCoverEventHandler cc_hdlr = getShortestPathCostCoverEventHandler(scip);
    return cc_hdlr.getNumConssAdded();
}

unsigned int getNDisjointPathsCostCoverCutsAdded(SCIP* scip) {
    CostCoverEventHandler cc_hdlr = getDisjointPathsCostCoverEventHandler(scip);
    return cc_hdlr.getNumConssAdded();
}
