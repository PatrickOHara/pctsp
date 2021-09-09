#include "pctsp/cost_cover.hh"
#include "pctsp/data_structures.hh"
#include "pctsp/logger.hh"

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
    BOOST_LOG_TRIVIAL(info) << nvars << " variables added to cover inequality.";
    for (int i = 0; i < nvars; i ++) {
        std::cout << SCIPvarGetName(variables[i]) << ": " << var_coefs[i] << std::endl;
        var_coefs[i] = 1;
    }
    double lhs = -SCIPinfinity(scip);
    double rhs = nvars - 1;
    SCIP_VAR** vars = variables.data();
    double* coefs = var_coefs.data();
    // SCIP_VAR* transvars[nvars];
    // SCIPgetTransformedVars(scip, nvars, vars, transvars);

    SCIP_CONS* cons;
    const char* cons_name = "cost-cover";
    SCIPcreateConsBasicLinear(scip, &cons, cons_name, nvars, vars, coefs, lhs, rhs);
    SCIPaddCons(scip, cons);
    SCIPreleaseCons(scip, &cons);

    // create the row and add the data and variables
    // BOOST_LOG_TRIVIAL(info) << "Adding variables to row.";
    // SCIP_ROW* row;
    // const char* row_name = "";
    // SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, row_name, lhs, rhs, false, false, true);
    // SCIPaddVarsToRow(scip, row, nvars, transvars, coefs);

    // BOOST_LOG_TRIVIAL(info) << "Added " << nvars << " to a cover inequality.";

    // // test if the cut effects the LP
    // if (SCIPisCutEfficacious(scip, sol, row)) {
    //     SCIP_Bool infeasible;
    //     SCIP_CALL(SCIPaddRow(scip, row, false, &infeasible));
    //     SCIPprintRow(scip, row, NULL);
    //     if (infeasible)
    //         *result = SCIP_CUTOFF;
    //     else
    //         *result = SCIP_SEPARATED;
    // }
    // SCIP_CALL(SCIPreleaseRow(scip, &row));
    return SCIP_OKAY;
}

// Cost cover event handler for new solutions

// struct SCIP_EventhdlrData
// {
//     std::vector<int>* path_distances;
// };

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
    // SCIP_EVENTHDLRDATA* eventhdlrdata = SCIPeventhdlrGetData(eventhdlr);
    double cost_upper_bound = SCIPgetUpperbound(scip);
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    SCIP_RESULT* result;
    PCTSPgraph graph = * probdata->getInputGraph();
    PCTSPvertex root_vertex = *probdata->getRootVertex();
    PCTSPedgeVariableMap edge_variable_map = *probdata->getEdgeVariableMap();

    // auto path_distances = *eventhdlrdata->path_distances;
    auto path_distances = _path_distances;
    auto violated_vertices = separateCostCoverPaths(graph, path_distances, cost_upper_bound);
    bool violation_found = violated_vertices.size() > 0;
    for (auto& vertex: violated_vertices) {
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
