#include "pctsp/cost_cover.hh"
#include "pctsp/data_structures.hh"
#include "pctsp/logger.hh"

struct SCIP_ConsData
{
    std::map<PCTSPvertex, int>* shortest_path_map;
    std::map<PCTSPvertex, int>* disjoint_paths_map;
};

SCIP_RETCODE addCoverInequality(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    std::vector<SCIP_VAR*>& variables,
    SCIP_RESULT* result,
    SCIP_SOL* sol
) {
    // at least one variable must be zero
    // x(S) <= |x(S)| - 1 
    int nvars = variables.size();
    std::vector<double> var_coefs(nvars);
    double lhs = -SCIPinfinity(scip);
    double rhs = nvars - 1;
    SCIP_VAR** vars = variables.data();
    double* coefs = var_coefs.data();
    SCIP_VAR* transvars[nvars];
    SCIPgetTransformedVars(scip, nvars, vars, transvars);

    // create the row and add the data and variables
    BOOST_LOG_TRIVIAL(info) << "Adding variables to row.";
    SCIP_ROW* row;
    const char* row_name = "";
    SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, row_name, lhs, rhs, false, false, true);
    SCIPaddVarsToRow(scip, row, nvars, transvars, coefs);

    BOOST_LOG_TRIVIAL(info) << "Added " << nvars << " to a cover inequality.";

    // test if the cut effects the LP
    if (SCIPisCutEfficacious(scip, sol, row)) {
        SCIP_Bool infeasible;
        SCIP_CALL(SCIPaddRow(scip, row, false, &infeasible));
        SCIPprintRow(scip, row, NULL);
        if (infeasible)
            *result = SCIP_CUTOFF;
        else
            *result = SCIP_SEPARATED;
    }
    SCIP_CALL(SCIPreleaseRow(scip, &row));
    return SCIP_OKAY;
}

SCIP_RETCODE createConsCostCover(
    SCIP*                 scip,               /**< SCIP data structure */
    SCIP_CONS**           cons,               /**< pointer to hold the created constraint */
    std::string& name,
    std::map<PCTSPvertex, int>& disjoint_paths_map,
    std::map<PCTSPvertex, int>& shortest_path_map,
    SCIP_Bool             initial,            /**< should the LP relaxation of constraint be in the initial LP? */
    SCIP_Bool             separate,           /**< should the constraint be separated during LP processing? */
    SCIP_Bool             enforce,            /**< should the constraint be enforced during node processing? */
    SCIP_Bool             check,              /**< should the constraint be checked for feasibility? */
    SCIP_Bool             propagate,          /**< should the constraint be propagated during node processing? */
    SCIP_Bool             local,              /**< is constraint only valid locally? */
    SCIP_Bool             modifiable,         /**< is constraint modifiable (subject to column generation)? */
    SCIP_Bool             dynamic,            /**< is constraint dynamic? */
    SCIP_Bool             removable           /**< should the constraint be removed from the LP due to aging or cleanup? */
) {
    SCIP_CONSHDLR* conshdlr;
    SCIP_CONSDATA* consdata;

    // find the constraint handler
    conshdlr = SCIPfindConshdlr(scip, "cost_cover");
    if (conshdlr == NULL)
    {
        SCIPerrorMessage("cost cover constraint handler not found\n");
        return SCIP_PLUGINNOTFOUND;
    }
    // store the constraint data
    SCIP_CALL( SCIPallocBlockMemory(scip, &consdata) );
    consdata->disjoint_paths_map = &disjoint_paths_map;
    consdata->shortest_path_map = &shortest_path_map;

    /* create constraint */
    SCIP_CALL(SCIPcreateCons(scip, cons, name.c_str(), conshdlr, consdata, initial, separate, enforce, check, propagate,
        local, modifiable, dynamic, removable, FALSE));

    return SCIP_OKAY;
}

SCIP_RETCODE checkAllCostCover(
    SCIP* scip,
    SCIP_CONSDATA* consdata,
    SCIP_RESULT* result,
    bool cost_cover_shortest_path,
    bool cost_cover_disjoint_paths,
    bool cost_cover_steiner_tree
) {
    auto cost_upper_bound = SCIPgetUpperbound(scip);
    bool shortest_path_violation = FALSE;
    bool disjoint_paths_violation = FALSE;

    if (cost_cover_shortest_path) {
        // ToDo: multiple shortest path by two
        auto shortest_path_map = *consdata->shortest_path_map;
        shortest_path_violation= isCostCoverPathsViolated(shortest_path_map, cost_upper_bound);
    }
    else if (cost_cover_disjoint_paths) {
        auto disjoint_paths_map = *consdata->disjoint_paths_map;
        disjoint_paths_violation = isCostCoverPathsViolated(disjoint_paths_map, cost_upper_bound);
    }
    else if (cost_cover_steiner_tree) {
        throw NotImplementedException();
    }
    if (shortest_path_violation || disjoint_paths_violation) {
        *result = SCIP_INFEASIBLE;
    }
    return SCIP_OKAY;
}

SCIP_DECL_CONSCHECK(CostCoverConshdlr::scip_check)
{
    
    bool found = FALSE;
    if (nconss == 0) {
        *result = SCIP_DIDNOTRUN;
    }
    else {
        SCIP_CONS* cons = conss[0];
        SCIP_CONSDATA* consdata = SCIPconsGetData(cons);
        checkAllCostCover(scip, consdata, result, cost_cover_shortest_path, cost_cover_disjoint_paths, cost_cover_steiner_tree);
   }
    return SCIP_OKAY;
}

SCIP_DECL_CONSENFOPS(CostCoverConshdlr::scip_enfops) {
    bool found = FALSE;
    if (nconss == 0) {
        *result = SCIP_DIDNOTRUN;
    }
    else {
        SCIP_CONS* cons = conss[0];
        SCIP_CONSDATA* consdata = SCIPconsGetData(cons);
        checkAllCostCover(scip, consdata, result, cost_cover_shortest_path, cost_cover_disjoint_paths, cost_cover_steiner_tree);
   }
    return SCIP_OKAY;
}

SCIP_DECL_CONSENFOLP(CostCoverConshdlr::scip_enfolp) {
    bool found = FALSE;
    if (nconss == 0) {
        *result = SCIP_DIDNOTRUN;
    }
    else {
        SCIP_CONS* cons = conss[0];
        SCIP_CONSDATA* consdata = SCIPconsGetData(cons);
        checkAllCostCover(scip, consdata, result, cost_cover_shortest_path, cost_cover_disjoint_paths, cost_cover_steiner_tree);
   }
    return SCIP_OKAY;
}

SCIP_DECL_CONSTRANS(CostCoverConshdlr::scip_trans) {
    return SCIP_OKAY;
}

SCIP_DECL_CONSLOCK(CostCoverConshdlr::scip_lock) {
    return SCIP_OKAY;
}

SCIP_DECL_CONSPRINT(CostCoverConshdlr::scip_print) {
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPALP(CostCoverConshdlr::scip_sepalp) {
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPASOL(CostCoverConshdlr::scip_sepasol) {
    return SCIP_OKAY;
}


// Cost cover event handler for new solutions

struct SCIP_EventhdlrData
{
    std::map<PCTSPvertex, int>* paths_cost_map;
};

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
    SCIP_EVENTHDLRDATA* eventhdlrdata = SCIPeventhdlrGetData(eventhdlr);
    double cost_upper_bound = SCIPgetUpperbound(scip);
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    SCIP_RESULT* result;
    PCTSPgraph graph = * probdata->getInputGraph();
    PCTSPvertex root_vertex = *probdata->getRootVertex();
    PCTSPedgeVariableMap edge_variable_map = *probdata->getEdgeVariableMap();

    auto paths_cost_map = *eventhdlrdata->paths_cost_map;
    auto violated_vertices = separateCostCoverPaths(paths_cost_map, cost_upper_bound);
    bool violation_found = violated_vertices.size() > 0;
    for (auto& vertex: violated_vertices) {
        std::vector<PCTSPvertex> cover_vertices = {root_vertex, vertex};
        addCoverInequalityFromVertices(scip, graph, cover_vertices, edge_variable_map, result, sol);
    }
    return SCIP_OKAY;
}

const std::string SHORTEST_PATH_COST_COVER_NAME = "shortest-path-cost-cover-event-handler";
const std::string DISJOINT_PATHS_COST_COVER_NAME = "disjoint-paths-cost-cover-event-handler";

template <typename Vertex>
SCIP_RETCODE includeCostCoverEventHandler(
    SCIP* scip,
    std::map<Vertex, int>& paths_cost_map
)
{
    CostCoverEventHandler* handler = new CostCoverEventHandler(scip);
    SCIP_CALL(SCIPincludeObjEventhdlr(scip, handler, true));

    SCIP_EVENTHDLRDATA* eventhdlrdata;   
    SCIP_CALL( SCIPallocBlockMemory(scip, &eventhdlrdata) );
    eventhdlrdata = SCIPeventhdlrGetData(handler);
    eventhdlrdata->paths_cost_map = paths_cost_map;
}