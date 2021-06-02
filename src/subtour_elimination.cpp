
#include <iostream>
#include "pctsp/separation.hh"
#include "pctsp/solution.hh"
#include "pctsp/subtour_elimination.hh"
#include "pctsp/exception.hh"
#include "pctsp/logger.hh"
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

std::vector<PCTSPedge> getInducedEdges(PCTSPgraph& graph, std::vector<PCTSPvertex>& vertices) {
    // assumes the graph is undirected!
    std::vector<PCTSPedge> edges;
    for (int i = 0; i < vertices.size(); i++) {
        for (int j = i + 1; j < vertices.size(); j++) {
            auto potential_edge = boost::edge(vertices[i], vertices[j], graph);
            if (potential_edge.second) {
                edges.push_back(potential_edge.first);
            }
        }
    }
    return edges;
}

SCIP_RETCODE addSubtourEliminationConstraint(
    SCIP* mip,
    SCIP_CONSHDLR* conshdlr,
    PCTSPgraph& graph,
    std::vector<PCTSPvertex>& vertex_set,
    PCTSPedgeVariableMap& edge_variable_map,
    PCTSPvertex& root_vertex,
    PCTSPvertex& target_vertex,
    SCIP_SOL* sol,                /**< primal solution that should be separated */
    SCIP_RESULT* result              /**< pointer to store the result of the separation call */
) {

    // assert that the root vertex is not in the set of vertices passed
    bool root_found = false;
    bool target_found = false;
    for (auto const& vertex : vertex_set) {
        if (vertex == root_vertex)
            root_found = true;
        if (vertex == target_vertex)
            target_found = true;
    }
    if (root_found) throw VertexInWrongSetException(std::to_string(root_vertex));
    if (!target_found) throw VertexInWrongSetException(std::to_string(target_vertex));

    // the name of the constraint contains every vertex in the set
    std::string cons_name = "sec-" + std::to_string(target_vertex);

    // get the set of edges contained in the subgraph induced over the vertex set
    std::vector<PCTSPedge> edge_vector = getInducedEdges(graph, vertex_set);
    VarVector edge_variables;
    for (auto const& edge : edge_vector) {
        edge_variables.push_back(edge_variable_map[edge]);
    }

    // get the variables of the vertices in the vertex set apart from v
    VarVector vertex_variables;
    for (auto const& vertex : vertex_set) {
        if (vertex != target_vertex) {  // target vertex not added to constraint
            auto potential_edge = boost::edge(vertex, vertex, graph);
            if (!potential_edge.second)
                throw NoSelfLoopFoundException(std::to_string(vertex));
            auto var = edge_variable_map[potential_edge.first];
            vertex_variables.push_back(var);
            // add the variable name to the constraint name
            cons_name += "-" + std::to_string(vertex);
        }
    }
    // x(E(S)) <= y(S) - y_v
    int nvars = edge_variables.size() + vertex_variables.size();
    BOOST_LOG_TRIVIAL(info) << edge_variables.size() << " edge variables and " << vertex_variables.size() << " vertex variables added to new constraint " << cons_name;

    // create an array of all variables of edges and vertices
    // the edges have positive coefficients and vertices have negative coefficients
    VarVector all_variables(nvars);
    std::vector<double> var_coefs(nvars);
    insertEdgeVertexVariables(edge_variables, vertex_variables, all_variables, var_coefs);

    // create the subtour elimination constraint
    // SCIP_VAR** vars = &all_variables[0];
    // double* vals = &var_coefs[0];
    double* vals = var_coefs.data();
    SCIP_VAR** vars = all_variables.data();
    for (int i = 0; i < nvars; i++)
        BOOST_LOG_TRIVIAL(debug) << i << ": " << vals[i] << SCIPvarGetName(vars[i]);
    double lhs = -SCIPinfinity(mip);
    double rhs = 0;
    // double lhs = 0;
    // double rhs = 7;

    // SCIP_CONS* cons;
    // SCIPcreateConsBasicLinear(mip, &cons, cons_name.c_str(), nvars, vars, vals, lhs, rhs);
    // SCIPcreateConsLinear(mip, &cons, cons_name.c_str(), nvars, vars, vals, lhs, rhs, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    // SCIPprintCons(mip, cons, NULL);
    // SCIPaddCons(mip, cons);
    // SCIPprintCons(mip, cons, NULL);
    // cout << "Num vars = " << SCIPgetNVarsLinear(mip, cons) << endl;

    SCIP_ROW* row;
    SCIP_CALL(SCIPcreateEmptyRowConshdlr(mip, &row, conshdlr, cons_name.c_str(), lhs, rhs, false, false, true));
    // SCIP_CALL(SCIPcreateRowConshdlr(mip, &row, conshdlr, cons_name.c_str(), lhs, rhs, false, false, true));
    SCIPprintRow(mip, row, NULL);
    // SCIP_CALL(SCIPaddVarsToRow(mip, row, nvars, vars, vals));
    SCIP_CALL(SCIPcacheRowExtensions(mip, row));
    for (int i = 0; i < nvars; i++) {
        SCIPaddVarToRow(mip, row, vars[i], vals[i]);
        SCIPprintRow(mip, row, NULL);
    }
    SCIP_CALL(SCIPflushRowExtensions(mip, row));

    cout << SCIProwGetVals(row) << endl;

    if (SCIPisCutEfficacious(mip, sol, row)) {
        SCIP_Bool infeasible;
        SCIPprintRow(mip, row, NULL);
        SCIP_CALL(SCIPaddRow(mip, row, false, &infeasible));
        SCIPprintRow(mip, row, NULL);
        if (infeasible)
            *result = SCIP_CUTOFF;
        else
            *result = SCIP_SEPARATED;
    }
    SCIP_CALL(SCIPreleaseRow(mip, &row));

    return SCIP_OKAY;
}

void insertEdgeVertexVariables(VarVector& edge_variables,
    VarVector& vertex_variables,
    VarVector& all_variables,
    std::vector<double>& var_coefs
) {
    BOOST_ASSERT(edge_variables.size() + vertex_variables.size() == all_variables.size());
    BOOST_ASSERT(all_variables.size() == var_coefs.size());
    for (int i = 0; i < edge_variables.size(); i++)
        all_variables[i] = edge_variables[i];
    for (int i = edge_variables.size(); i < all_variables.size(); i++)
        all_variables[i] = vertex_variables[i - edge_variables.size()];
    auto coef_it = var_coefs.begin();
    std::advance(coef_it, edge_variables.size());
    std::fill(var_coefs.begin(), coef_it, 1);
    std::fill(coef_it, var_coefs.end(), -1);
}

SCIP_RETCODE PCTSPcreateConsSubtour(
    SCIP* mip,
    SCIP_CONS** cons,
    std::string& name,
    PCTSPgraph& graph,
    PCTSPvertex& root_vertex,
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
    // PCTSPsubtourEliminationData* consdata;
    SCIP_CONSDATA* consdata;

    /* find the subtour constraint handler */
    conshdlr = SCIPfindConshdlr(mip, "subtour");
    if (conshdlr == NULL)
    {
        SCIPerrorMessage("subtour constraint handler not found\n");
        return SCIP_PLUGINNOTFOUND;
    }
    /* create constraint */
    SCIP_CALL(SCIPcreateCons(mip, cons, name.c_str(), conshdlr, consdata, initial, separate, enforce, check, propagate,
        local, modifiable, dynamic, removable, FALSE));

    return SCIP_OKAY;
}

SCIP_RETCODE PCTSPcreateBasicConsSubtour(
    SCIP* mip,
    SCIP_CONS** cons,
    std::string& name,
    PCTSPgraph& graph,
    PCTSPvertex& root_vertex
) {
    SCIP_CALL(PCTSPcreateConsSubtour(
        mip,
        cons,
        name,
        graph,
        root_vertex,
        FALSE,
        TRUE,
        TRUE,
        TRUE,
        TRUE,
        FALSE,
        FALSE,
        FALSE,
        FALSE    // ToDo: later we may be able to remove this due to cleanup
    ));
    return SCIP_OKAY;
}

PCTSPgraph* ProbDataPCTSP::getInputGraph() {
    return graph_;
}

int* ProbDataPCTSP::getQuota() {
    return quota_;
}

PCTSPvertex* ProbDataPCTSP::getRootVertex() {
    return root_vertex_;
}

PCTSPedgeVariableMap* ProbDataPCTSP::getEdgeVariableMap() {
    return edge_variable_map_;
}

SCIP_DECL_CONSCHECK(PCTSPconshdlrSubtour::scip_check)
{
    BOOST_LOG_TRIVIAL(debug) << "Checking for subtours.";
    assert(result != NULL);
    *result = SCIP_FEASIBLE;

    assert(sol != NULL);
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    auto& graph = *probdata->getInputGraph();
    auto& edge_variable_map = *probdata->getEdgeVariableMap();

    PCTSPgraph solution_graph;
    getSolutionGraph(scip, graph, solution_graph, sol, edge_variable_map);
    BOOST_LOG_TRIVIAL(debug) << "Solution graph has " << boost::num_vertices(solution_graph) << " vertices and " << boost::num_edges(solution_graph) << " edges.";

    // if a subtour is found, the solution must be infeasible
    std::vector< int > component(boost::num_vertices(solution_graph));
    bool is_simple_cycle = isGraphSimpleCycle(solution_graph, component);
    if (!is_simple_cycle) {
        BOOST_LOG_TRIVIAL(info) << "Violation: support graph is not a simple cycle. Return Infeasible.";
        *result = SCIP_INFEASIBLE;
    }
    else
        BOOST_LOG_TRIVIAL(info) << "Solution is a simple cycle. No subtour violations found.";
    return SCIP_OKAY;
}

SCIP_DECL_CONSENFOPS(PCTSPconshdlrSubtour::scip_enfops) {
    BOOST_LOG_TRIVIAL(debug) << "SCIP enfops method";
    return SCIP_OKAY;
}
SCIP_DECL_CONSENFOLP(PCTSPconshdlrSubtour::scip_enfolp) {
    BOOST_LOG_TRIVIAL(debug) << "SCIP enfolp method";
    return SCIP_OKAY;
}

SCIP_DECL_CONSTRANS(PCTSPconshdlrSubtour::scip_trans) {
    BOOST_LOG_TRIVIAL(debug) << "SCIP trans method";
    SCIP_CONSDATA* sourcedata;
    SCIP_CONSDATA* targetdata = NULL;

    /* create target constraint */
    SCIP_CALL(SCIPcreateCons(scip, targetcons, SCIPconsGetName(sourcecons), conshdlr, targetdata,
        SCIPconsIsInitial(sourcecons), SCIPconsIsSeparated(sourcecons), SCIPconsIsEnforced(sourcecons),
        SCIPconsIsChecked(sourcecons), SCIPconsIsPropagated(sourcecons), SCIPconsIsLocal(sourcecons),
        SCIPconsIsModifiable(sourcecons), SCIPconsIsDynamic(sourcecons), SCIPconsIsRemovable(sourcecons),
        SCIPconsIsStickingAtNode(sourcecons)));
    return SCIP_OKAY;
}

SCIP_DECL_CONSLOCK(PCTSPconshdlrSubtour::scip_lock) {
    BOOST_LOG_TRIVIAL(debug) << "SCIP lock method";
    return SCIP_OKAY;
}

SCIP_DECL_CONSPRINT(PCTSPconshdlrSubtour::scip_print) {
    SCIP_Bool success = false;
    int nvars = 0;
    SCIPgetConsNVars(scip, cons, &nvars, &success);
    if (nvars == 0) {
        SCIPinfoMessage(scip, file, "No variables found for SEC");
    }
    else {
        SCIP_VAR* vars[nvars];
        SCIPgetConsVars(scip, cons, vars, nvars, &success);
        std::string message = std::to_string(SCIPconsGetLhs(scip, cons, &success));
        message += " <= ";
        for (int i = 0; i < nvars; i++) {
            message += SCIPvarGetName(vars[i]);
            message += " + ";
        }
        message += " <= ";
        message += std::to_string(SCIPconsGetRhs(scip, cons, &success));
        message += "\n";
        SCIPinfoMessage(scip, file, message.c_str());

    }
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPALP(PCTSPconshdlrSubtour::scip_sepalp) {
    BOOST_LOG_TRIVIAL(debug) << "scip_sepalp";
    SCIP_CALL(PCTSPseparateSubtour(scip, conshdlr, NULL, result));
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPASOL(PCTSPconshdlrSubtour::scip_sepasol) {
    BOOST_LOG_TRIVIAL(debug) << "scip_sepsol";
    SCIP_CALL(PCTSPseparateSubtour(scip, conshdlr, sol, result));
    return SCIP_OKAY;
}

SCIP_RETCODE PCTSPseparateSubtour(
    SCIP* scip,               /**< SCIP data structure */
    SCIP_CONSHDLR* conshdlr,           /**< the constraint handler itself */
    SCIP_SOL* sol,                /**< primal solution that should be separated */
    SCIP_RESULT* result              /**< pointer to store the result of the separation call */
) {
    *result = SCIP_DIDNOTFIND;
    // load the constraint handler data
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    auto& graph = *(probdata->getInputGraph());
    auto& edge_variable_map = *(probdata->getEdgeVariableMap());
    PCTSPgraph solution_graph;
    getSolutionGraph(scip, graph, solution_graph, sol, edge_variable_map);
    auto& root_vertex = *(probdata->getRootVertex());

    // get the connected components of the support graph
    BOOST_LOG_TRIVIAL(debug) << "Finding connected components of the solution graph.";
    std::vector< int > component(boost::num_vertices(solution_graph));
    int n_components = boost::connected_components(solution_graph, &component[0]);
    if (n_components == 1) return SCIP_OKAY;
    std::vector<std::vector<PCTSPvertex>> component_sets;
    for (int i = 0; i < n_components; i++)
        component_sets.push_back(std::vector<PCTSPvertex>());
    auto index = boost::get(vertex_index, solution_graph);
    int root_component = -1;
    for (auto vertex : boost::make_iterator_range(boost::vertices(solution_graph))) {
        int component_id = component[index[vertex]];
        component_sets[component_id].push_back(vertex);
        if (vertex == root_vertex)
            root_component = component_id;
    }
    assert(root_component >= 0);
    // for each vertex in a connected component C that does not contain the root
    // add a subtour elimination constraint over the vertex and C
    for (auto vertex : boost::make_iterator_range(boost::vertices(solution_graph))) {
        int component_id = component[index[vertex]];
        if (component_id != root_component) {
            BOOST_LOG_TRIVIAL(debug) << "Add subtour elimination constraint to connected component " << component_id << " for vertex " << vertex;
            std::vector<PCTSPvertex> support_vertex_set = component_sets[component_id];
            std::vector<PCTSPvertex> vertex_set(support_vertex_set.size());
            int i = 0;
            for (auto const& solution_vertex : support_vertex_set) {
                vertex_set[i] = boost::vertex(solution_vertex, graph);
                i++;
            }
            auto target_vertex = boost::vertex(vertex, graph);
            SCIP_CALL(addSubtourEliminationConstraint(
                scip,
                conshdlr,
                graph,
                vertex_set,
                edge_variable_map,
                root_vertex,
                target_vertex,
                sol,
                result
            ));
        }
    }
    BOOST_LOG_TRIVIAL(debug) << "Result is " << *result;
    return SCIP_OKAY;
}