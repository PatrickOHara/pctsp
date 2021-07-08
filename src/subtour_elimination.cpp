
#include <iostream>
#include "pctsp/separation.hh"
#include "pctsp/solution.hh"
#include "pctsp/subtour_elimination.hh"
#include "pctsp/exception.hh"
#include "pctsp/logger.hh"
#include <boost/graph/push_relabel_max_flow.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/strong_components.hpp>
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


/* Get the number of variables that are fixed or aggregated.
 * You should pass the transformed variables via the array.
 */
int numFixedOrAggVars(SCIP_VAR** vars, int nvars) {
    int count = 0;
    for (int i = 0; i < nvars; i++) {
        auto var = vars[i];
        switch (SCIPvarGetStatus(var)) {
        case SCIP_Varstatus::SCIP_VARSTATUS_FIXED:
        case SCIP_Varstatus::SCIP_VARSTATUS_AGGREGATED: {
            count += 1;
            break;
        }
        default: break;
        }
    }
    return count;
}

bool isSolSimpleCycle(SCIP* scip, SCIP_SOL* sol, SCIP_RESULT* result) {
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    auto& graph = *probdata->getInputGraph();
    auto& edge_variable_map = *probdata->getEdgeVariableMap();
    auto solution_edges = getSolutionEdges(scip, graph, sol, edge_variable_map);
    logSolutionEdges(scip, graph, sol, edge_variable_map);
    return isSimpleCycle(graph, solution_edges);
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
    std::string debug_message;
    for (auto const& edge : edge_vector) {
        edge_variables.push_back(edge_variable_map[edge]);
        debug_message += std::to_string(boost::source(edge, graph)) + "-" + std::to_string(boost::target(edge, graph)) + " ";
    }
    std::string vertex_message;
    for (auto const& vertex : vertex_set)
        vertex_message += std::to_string(vertex) + " ";
    BOOST_LOG_TRIVIAL(debug) << "Vertex " << target_vertex << " in component with vertices " << vertex_message;
    BOOST_LOG_TRIVIAL(debug) << "Edges to add: " << debug_message;

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
    BOOST_LOG_TRIVIAL(debug) << edge_variables.size() << " edge variables and " << vertex_variables.size() << " vertex variables added to new constraint " << cons_name;

    // create an array of all variables of edges and vertices
    // the edges have positive coefficients and vertices have negative coefficients
    VarVector all_variables(nvars);
    std::vector<double> var_coefs(nvars);
    insertEdgeVertexVariables(edge_variables, vertex_variables, all_variables, var_coefs);

    // create the subtour elimination constraint
    double* vals = var_coefs.data();
    SCIP_VAR** vars = all_variables.data();
    double lhs = -SCIPinfinity(mip);
    double rhs = 0;

    SCIP_CONS* cons;

    SCIP_VAR* transvars[nvars];

    for (int i = 0; i < nvars; i++) {
        SCIP_VAR* transvar;
        SCIP_CALL(SCIPgetTransformedVar(mip, vars[i], &transvar));
        transvars[i] = transvar;
    }

    SCIP_ROW* row;
    SCIP_CALL(SCIPcreateEmptyRowConshdlr(mip, &row, conshdlr, cons_name.c_str(), lhs, rhs, false, false, true));
    SCIP_CALL(SCIPcacheRowExtensions(mip, row));

    for (int i = 0; i < nvars; i++) {
        auto transvar = transvars[i];
        SCIPaddVarToRow(mip, row, transvar, vals[i]);
    }
    SCIP_CALL(SCIPflushRowExtensions(mip, row));

    if (SCIPisCutEfficacious(mip, sol, row)) {
        SCIP_Bool infeasible;
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
        TRUE    // ToDo: later we may be able to remove this due to cleanup
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
    auto nvars = SCIPgetNVars(scip);
    SCIP_VAR* transvars[nvars];
    SCIPgetTransformedVars(scip, nvars, SCIPgetVars(scip), transvars);
    auto nfixed = numFixedOrAggVars(transvars, nvars);
    BOOST_LOG_TRIVIAL(debug) << "scip_check: Checking for subtours. " << nfixed << " fixed/agg vars out of " << nvars;
    *result = SCIP_FEASIBLE;
    if (isSolSimpleCycle(scip, sol, result))
        BOOST_LOG_TRIVIAL(info) << "Solution is a simple cycle. No subtour violations found.";
    else {
        BOOST_LOG_TRIVIAL(debug) << "Violation: support graph is not a simple cycle. Return Infeasible.";
        *result = SCIP_INFEASIBLE;
    }
    return SCIP_OKAY;
}

SCIP_DECL_CONSENFOPS(PCTSPconshdlrSubtour::scip_enfops) {
    if (isSolSimpleCycle(scip, NULL, result)) {
        BOOST_LOG_TRIVIAL(debug) << "SCIP enfops: solution is simple cycle";
        *result = SCIP_FEASIBLE;
    }
    else {
        BOOST_LOG_TRIVIAL(debug) << "SCIP enfops: solution is not simple cycle";
        *result = SCIP_INFEASIBLE;
    }
    return SCIP_OKAY;
}

SCIP_DECL_CONSENFOLP(PCTSPconshdlrSubtour::scip_enfolp) {
    if (isSolSimpleCycle(scip, NULL, result)) {
        BOOST_LOG_TRIVIAL(debug) << "SCIP enfolp: LP is simple cycle";
        *result = SCIP_FEASIBLE;
    }
    else {
        BOOST_LOG_TRIVIAL(debug) << "SCIP enfolp: LP is not simple cycle";
        *result = SCIP_INFEASIBLE;
    }
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
    SCIP_CALL(PCTSPseparateSubtour(scip, conshdlr, conss, nconss, nusefulconss, NULL, result));
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPASOL(PCTSPconshdlrSubtour::scip_sepasol) {
    BOOST_LOG_TRIVIAL(debug) << "scip_sepsol";
    SCIP_CALL(PCTSPseparateSubtour(scip, conshdlr, conss, nconss, nusefulconss, sol, result));
    return SCIP_OKAY;
}

SCIP_RETCODE PCTSPseparateDisjointTour(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    PCTSPgraph& input_graph,
    PCTSPgraph& support_graph,
    PCTSPedgeVariableMap& edge_variable_map,
    PCTSPvertex& root_vertex,
    SCIP_SOL* sol,
    SCIP_RESULT* result,
    std::vector<int>& component,
    int& n_components,
    int& root_component,
    int freq
) {
    // get the connected components of the support graph
    n_components = boost::connected_components(support_graph, &component[0]);
    if (n_components == 1) {
        root_component = 1;
        return SCIP_OKAY;
    }
    std::vector<std::vector<PCTSPvertex>> component_sets;
    for (int i = 0; i < n_components; i++)
        component_sets.push_back(std::vector<PCTSPvertex>());
    auto index = boost::get(vertex_index, support_graph);
    root_component = -1;
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
        std::vector<PCTSPvertex> support_vertex_set = component_sets[component_id];
        if (component_id != root_component && support_vertex_set.size() >= 3) {
            std::vector<PCTSPvertex> vertex_set(support_vertex_set.size());
            int i = 0;
            // get the vertex objects from the original graph
            for (auto const& solution_vertex : support_vertex_set) {
                vertex_set[i] = boost::vertex(solution_vertex, input_graph);
                i++;
            }
            auto target_vertex = boost::vertex(vertex, input_graph);
            SCIP_CALL(addSubtourEliminationConstraint(
                scip,
                conshdlr,
                input_graph,
                vertex_set,
                edge_variable_map,
                root_vertex,
                target_vertex,
                sol,
                result
            ));
        }

    }
    return SCIP_OKAY;
}

SCIP_RETCODE PCTSPseparateMaxflowMincut(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    PCTSPgraph& input_graph,
    PCTSPedgeVariableMap& edge_variable_map,
    PCTSPvertex& root_vertex,
    SCIP_SOL* sol,
    SCIP_RESULT* result,
    std::set<PCTSPvertex>& root_component,
    int freq
) {
    DirectedCapacityGraph support_graph;
    // create a capacity map for the edges
    // the capacity of each edge is the value in the solution given
    auto solution_edges = getSolutionEdges(scip, input_graph, sol, edge_variable_map);
    CapacityVector capacity_vector = getCapacityVectorFromSol(scip, input_graph, sol, edge_variable_map);

    // only get edges that are in the same connected components as the root
    VertexPairVector edge_vector = getVertexPairVectorFromEdgeSubset(input_graph, solution_edges);
    VertexPairVector root_edge_vector;
    for (auto const& pair : edge_vector) {
        if (root_component.count(pair.first) == 1 && root_component.count(pair.second == 1)) {
            root_edge_vector.push_back(pair);
        }
    }
    // create mapping from input vertices to support vertices (they will be renamed)
    typedef boost::graph_traits<DirectedCapacityGraph>::vertex_descriptor SupportVertex;
    boost::bimap<SupportVertex, PCTSPvertex> lookup;
    auto support_edges = renameEdges(lookup, root_edge_vector);

    // create the directed support graph with an edge attribute for the capacity
    auto capacity_property = boost::get(edge_capacity, support_graph);
    auto reverse_edges = boost::get(edge_reverse, support_graph);
    auto residual_capacity = boost::get(edge_residual_capacity, support_graph);
    for (int i = 0; i < support_edges.size(); i++) {
        auto pair = support_edges[i];
        auto cap = capacity_vector[i];

        // add edge in both directions since the graph is undirected
        auto edge1 = boost::add_edge(pair.first, pair.second, cap, support_graph);
        auto edge2 = boost::add_edge(pair.second, pair.first, cap, support_graph);

        // add a reverse edge
        auto edge1_reverse = boost::add_edge(pair.second, pair.first, 0, support_graph);
        auto edge2_reverse = boost::add_edge(pair.first, pair.second, 0, support_graph);

        // store the reverse edges
        reverse_edges[edge1.first] = edge1_reverse.first;
        reverse_edges[edge1_reverse.first] = edge1.first;
        reverse_edges[edge2.first] = edge2_reverse.first;
        reverse_edges[edge2_reverse.first] = edge2.first;
    }

    auto support_root = getNewVertex(lookup, root_vertex);
    auto vindex = boost::get(vertex_index, support_graph);
    std::vector< int > component(boost::num_vertices(support_graph));
    int num = strong_components(support_graph, make_iterator_property_map(component.begin(), get(vertex_index, support_graph)));
    BOOST_LOG_TRIVIAL(debug) << "Directed capacity graph has " << boost::num_edges(support_graph) << " edges and " << num << " strongly connected components.";
    BOOST_ASSERT(num <= 1);
    for (auto target : boost::make_iterator_range(boost::vertices(support_graph))) {
        if (target != support_root) {
            // reset the residual capacity
            for (auto edge : boost::make_iterator_range(boost::edges(support_graph))) {
                residual_capacity[edge] = 0;
            }
            auto flow = boost::push_relabel_max_flow(support_graph, support_root, target, capacity_property, residual_capacity, reverse_edges, vindex);

            if (flow < 2 * FLOW_FLOAT_MULTIPLIER) {
                BOOST_LOG_TRIVIAL(info) << "Flow constraint violated: flow from " << support_root << " to " << target << " is: " << flow;

                // get the flow of each edge
                // auto eindex = boost::get(boost::edge_index, support_graph);
                // auto edge_flow = boost::make_vector_property_map<boost::edge_weight_t>(eindex);

                // traverse the residual graph from the root
                // all vertices that are reachable on edges that have some flow are on one side of the cut
                // all edges that are not reachable by the flow are on the other side of the cut
                auto unreachable = getUnreachableVertices(support_graph, support_root, residual_capacity);
                BOOST_LOG_TRIVIAL(info) << unreachable.size() << " vertices are unreachable from root of the residual graph.";
                auto unreachable_input_vertices = getOldVertices(lookup, unreachable);
                auto input_target_vertex = getOldVertex(lookup, target);
                // the component not containing the root violates the subtour elimination constraint
                SCIP_CALL(addSubtourEliminationConstraint(
                    scip,
                    conshdlr,
                    input_graph,
                    unreachable_input_vertices,
                    edge_variable_map,
                    root_vertex,
                    input_target_vertex,
                    sol,
                    result
                ));
            }
        }
    }
    // get the component containing the root
    // bool root_component_id = boost::get(parities, support_root);
    // PCTSPvertexVector root_component;
    // PCTSPvertexVector non_root_component; // get the component not containing the root
    // for (auto vertex : boost::make_iterator_range(boost::vertices(support_graph))) {
    //     if (boost::get(parities, vertex) == root_component_id)
    //         root_component.push_back(vertex);
    //     else
    //         non_root_component.push_back(vertex);
    // }

    // we obtain a cut (S_v, V* / S_v)
    // the set S_v is a possibly violated inequality x(E(S)) <= y(S) - y_v

    // if S_v violates inequality then add it to the LP
    // PCTSPvertex target_vertex;
    // std::vector<PCTSPvertex> support_vertex_set;
    // if (support_vertex_set.size() >= 3) {
    //     std::vector<PCTSPvertex> vertex_set(support_vertex_set.size());
    //     int i = 0;
    //     // get the vertex objects from the original graph
    //     for (auto const& solution_vertex : support_vertex_set) {
    //         vertex_set[i] = boost::vertex(solution_vertex, input_graph);
    //         i++;
    //     }
    //     SCIP_CALL(addSubtourEliminationConstraint(
    //         scip,
    //         conshdlr,
    //         input_graph,
    //         vertex_set,
    //         edge_variable_map,
    //         root_vertex,
    //         target_vertex,
    //         sol,
    //         result
    //     ));
    // }
    return SCIP_OKAY;
}

SCIP_RETCODE PCTSPseparateSubtour(
    SCIP* scip,               /**< SCIP data structure */
    SCIP_CONSHDLR* conshdlr,           /**< the constraint handler itself */
    SCIP_CONS** conss,              /**< array of constraints to process */
    int nconss,             /**< number of constraints to process */
    int nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
    SCIP_SOL* sol,                /**< primal solution that should be separated */
    SCIP_RESULT* result              /**< pointer to store the result of the separation call */
) {
    *result = SCIP_DIDNOTFIND;
    // load the constraint handler data
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    auto& input_graph = *(probdata->getInputGraph());
    auto& edge_variable_map = *(probdata->getEdgeVariableMap());
    auto& root_vertex = *(probdata->getRootVertex());

    bool sec_disjoint_tour = true;
    int sec_disjoint_tour_freq = 1;
    bool sec_maxflow_mincut = true;
    int sec_maxflow_mincut_freq = 1;
    if (sec_disjoint_tour && sec_maxflow_mincut_freq > 0) {
        PCTSPgraph support_graph;
        getSolutionGraph(scip, input_graph, support_graph, sol, edge_variable_map);
        std::vector< int > component(boost::num_vertices(support_graph));
        int n_components;
        int root_component_id;
        PCTSPseparateDisjointTour(
            scip, conshdlr, input_graph, support_graph, edge_variable_map, root_vertex, sol, result, component, n_components, root_component_id, sec_disjoint_tour_freq
        );
        if (sec_maxflow_mincut && sec_maxflow_mincut_freq > 0)
        {
            std::set<PCTSPvertex> root_component;
            auto v_index = boost::get(vertex_index, support_graph);
            for (auto vertex : boost::make_iterator_range(boost::vertices(support_graph))) {
                if (component[v_index[vertex]] == root_component_id) {
                    root_component.emplace(vertex);
                }
            }
            PCTSPseparateMaxflowMincut(
                scip, conshdlr, input_graph, edge_variable_map, root_vertex, sol, result, root_component, sec_maxflow_mincut_freq
            );
        }
    }
    return SCIP_OKAY;
}
