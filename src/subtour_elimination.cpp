
#include "pctsp/separation.hh"
#include "pctsp/subtour_elimination.hh"
#include "pctsp/exception.hh"
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

std::vector<PCTSPedge> getInducedEdges(PCTSPgraph& graph, std::vector<PCTSPvertex>& vertices) {
    std::vector<PCTSPedge> edges;
    for (auto const& u : vertices) {
        for (auto const& v : vertices) {
            auto potential_edge = boost::edge(u, v, graph);
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
    std::string cons_name = "sec-";

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
            cons_name += std::string(SCIPvarGetName(var));
        }
    }
    // x(E(S)) <= y(S) - y_v
    int nvars = edge_variables.size() + vertex_variables.size();

    // create an array of all variables of edges and vertices
    // the edges have positive coefficients and vertices have negative coefficients
    VarVector all_variables(nvars);
    std::vector<double> var_coefs(nvars);
    insertEdgeVertexVariables(edge_variables, vertex_variables, all_variables, var_coefs);

    // create the subtour elimination constraint
    SCIP_CONS* cons = nullptr;
    SCIP_VAR** vars = &all_variables[0];
    double* vals = &var_coefs[0];
    double lhs = -SCIPinfinity(mip);
    double rhs = 0;
    SCIP_ROW* row;
    SCIP_CALL(SCIPcreateEmptyRowConshdlr(mip, &row, conshdlr, cons_name.c_str(), lhs, rhs, false, false, true));
    // SCIP_CALL(SCIPcreateConsBasicLinear(mip, &cons, cons_name.c_str(), nvars, vars, vals, lhs, rhs));

    SCIP_CALL(SCIPaddVarsToRow(mip, row, nvars, vars, vals));
    SCIP_Bool infeasible;
    SCIP_CALL(SCIPaddRow(mip, row, false, &infeasible));
    if (infeasible)
        *result = SCIP_CUTOFF;
    else
        *result = SCIP_SEPARATED;
    // add the constraint to the solver
    // SCIP_CALL(SCIPaddCons(mip, cons));
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

SCIP_DECL_CONSCHECK(PCTSPconshdlrSubtour::scip_check)
{
    assert(result != NULL);
    *result = SCIP_FEASIBLE;
    // conshdlr = SCIPfindConshdlr(scip, "subtour");
    SCIP_ConshdlrData* consdata = SCIPconshdlrGetData(conshdlr);
    PCTSPgraph* graph = consdata->graph;
    auto edge_variable_map = consdata->edge_variable_map;
    auto support_graph = getSolutionGraph(scip, *graph, sol, *edge_variable_map);

    // if a subtour is found, the solution must be infeasible
    std::vector< int > component(boost::num_vertices(support_graph));
    bool is_simple_cycle = isGraphSimpleCycle(support_graph, component);
    if (!is_simple_cycle) {
        *result = SCIP_INFEASIBLE;
        if (printreason)
            SCIPinfoMessage(scip, NULL, "violation: graph has a subtour\n");
    }
    return SCIP_OKAY;
}

SCIP_DECL_CONSENFOPS(PCTSPconshdlrSubtour::scip_enfops) {
    return SCIP_OKAY;
}
SCIP_DECL_CONSENFOLP(PCTSPconshdlrSubtour::scip_enfolp) {
    return SCIP_OKAY;
}
SCIP_DECL_CONSTRANS(PCTSPconshdlrSubtour::scip_trans) {
    return SCIP_OKAY;
}

SCIP_DECL_CONSLOCK(PCTSPconshdlrSubtour::scip_lock) {
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPALP(PCTSPconshdlrSubtour::scip_sepalp) {
    SCIP_CALL(PCTSPseparateSubtour(scip, conshdlr, NULL, result));
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPASOL(PCTSPconshdlrSubtour::scip_sepasol) {
    SCIP_CALL(PCTSPseparateSubtour(scip, conshdlr, sol, result));
    return SCIP_OKAY;
}

SCIP_RETCODE PCTSPseparateSubtour(
    SCIP* scip,               /**< SCIP data structure */
    SCIP_CONSHDLR* conshdlr,           /**< the constraint handler itself */
    SCIP_SOL* sol,                /**< primal solution that should be separated */
    SCIP_RESULT* result              /**< pointer to store the result of the separation call */
) {
    *result = SCIP_FEASIBLE;
    // conshdlr = SCIPfindConshdlr(scip, "subtour");

    // load the constraint handler data
    SCIP_ConshdlrData* consdata = SCIPconshdlrGetData(conshdlr);
    PCTSPgraph* graph = consdata->graph;
    auto edge_variable_map = consdata->edge_variable_map;
    auto support_graph = getSolutionGraph(scip, *graph, sol, *edge_variable_map);
    auto root_vertex = *consdata->root_vertex;

    // get the connected components of the support graph
    std::vector< int > component(boost::num_vertices(support_graph));
    int n_components = boost::connected_components(support_graph, &component[0]);
    if (n_components == 1) return SCIP_OKAY;
    std::vector<std::vector<PCTSPvertex>> component_sets;
    auto index = boost::get(vertex_index, support_graph);
    int root_component = -1;
    for (auto vertex : boost::make_iterator_range(boost::vertices(support_graph))) {
        int component_id = component[index[vertex]];
        component_sets[component_id].push_back(vertex);
        if (vertex == root_vertex)
            root_component = component_id;
    }
    assert(root_component >= 0);
    // for each vertex in a connected component C that does not contain the root
    // add a subtour elimination constraint over the vertex and C
    for (auto vertex : boost::make_iterator_range(boost::vertices(support_graph))) {
        int component_id = component[index[vertex]];
        if (component_id != root_component) {
            std::vector<PCTSPvertex> vertex_set = component_sets[component_id];
            SCIP_CALL(addSubtourEliminationConstraint(
                scip,
                conshdlr,
                *graph,
                vertex_set,
                *edge_variable_map,
                root_vertex,
                vertex,
                NULL,
                result
            ));
        }
    }
    return SCIP_OKAY;
}