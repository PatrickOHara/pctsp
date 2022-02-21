
#include <iostream>
#include "pctsp/separation.hh"
#include "pctsp/solution.hh"
#include "pctsp/subtour_elimination.hh"
#include "pctsp/exception.hh"
#include "pctsp/logger.hh"
#include "pctsp/event_handlers.hh"
#include <boost/graph/push_relabel_max_flow.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/strong_components.hpp>
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

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
    return isSimpleCycle(graph, solution_edges);
}

SCIP_RETCODE addSubtourEliminationConstraint(
    SCIP* scip,
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
    // either the target or the root vertex should be in the vertex set
    if (! (root_found || target_found)) {
        throw VertexNotFoundException(std::to_string(target_vertex));
    }
    // but the target and root should not be in the same vertex set
    if (root_found && target_found) {
        throw VertexInWrongSetException(std::to_string(target_vertex));
    }

    // get the set of edges contained in the subgraph induced over the vertex set
    std::vector<PCTSPedge> edge_vector = getEdgesInducedByVertices(graph, vertex_set);
    VarVector edge_variables = getEdgeVariables(scip, graph, edge_variable_map, edge_vector);

    // get vertex variables
    std::vector<PCTSPvertex> vertices_to_find;
    for (auto const & vertex : vertex_set) {
        // if the root is found in the vertex set, then add all vertices
        // if the target is found in the vertex set, then add all vertices except the target vertex
        if (root_found || (target_found && vertex != target_vertex)) {
            vertices_to_find.push_back(vertex);
        }
    }
    // if the root is found, we also need to add the target vertex variable
    if (root_found) vertices_to_find.push_back(target_vertex);
    auto self_loops = getSelfLoops(graph, vertices_to_find);
    auto vertex_variables = getEdgeVariables(scip, graph, edge_variable_map, self_loops);

    // x(E(S)) <= y(S) - y_v
    int nvars = edge_variables.size() + vertex_variables.size();

    // create an array of all variables of edges and vertices
    // the edges have positive coefficients and vertices have negative coefficients
    VarVector all_vars(nvars);
    std::vector<double> var_coefs(nvars);
    fillPositiveNegativeVars(edge_variables, vertex_variables, all_vars, var_coefs);

    // if the root vertex is found, then we must change the value of the target coef variable to be positive
    if (root_found) var_coefs[var_coefs.size()-1] = 1.0;

    // the name of the constraint contains every vertex in the set
    std::string cons_name = "SubtourElimination_" + joinVariableNames(all_vars);
    BOOST_LOG_TRIVIAL(debug) << edge_variables.size() << " edge variables and " << vertex_variables.size() << " vertex variables added to new subtour elimination constraint.";

    // create the subtour elimination constraint
    double lhs = -SCIPinfinity(scip);
    double rhs = 0;
    return addRow(scip, conshdlr, result, sol, all_vars, var_coefs, lhs, rhs, cons_name);
}
 
SCIP_RETCODE PCTSPcreateConsSubtour(
    SCIP* scip,
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
    conshdlr = SCIPfindConshdlr(scip, SEC_CONSHDLR_NAME.c_str());
    if (conshdlr == NULL)
    {
        SCIPerrorMessage("subtour constraint handler not found\n");
        return SCIP_PLUGINNOTFOUND;
    }
    /* create constraint */
    SCIP_CALL(SCIPcreateCons(scip, cons, name.c_str(), conshdlr, consdata, initial, separate, enforce, check, propagate,
        local, modifiable, dynamic, removable, FALSE));

    return SCIP_OKAY;
}

SCIP_RETCODE PCTSPcreateBasicConsSubtour(
    SCIP* scip,
    SCIP_CONS** cons,
    std::string& name,
    PCTSPgraph& graph,
    PCTSPvertex& root_vertex
) {
    SCIP_CALL(PCTSPcreateConsSubtour(
        scip,
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

SCIP_DECL_CONSCHECK(PCTSPconshdlrSubtour::scip_check)
{
    auto nvars = SCIPgetNVars(scip);
    SCIP_VAR* transvars[nvars];
    SCIPgetTransformedVars(scip, nvars, SCIPgetVars(scip), transvars);
    auto nfixed = numFixedOrAggVars(transvars, nvars);
    BOOST_LOG_TRIVIAL(debug) << "scip_check: Checking for subtours. " << nfixed << " fixed/agg vars out of " << nvars;
    BOOST_LOG_TRIVIAL(debug) << "LP objective value: " << SCIPgetLPObjval(scip) << ". Solution value: " << SCIPsolGetOrigObj(sol);
    if (isSolSimpleCycle(scip, sol, result)) {
        BOOST_LOG_TRIVIAL(debug) << "Solution is a simple cycle. No subtour violations found.";
        *result = SCIP_FEASIBLE;
    }
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
        SCIP_NODE* node = SCIPgetCurrentNode(scip);
        // double lower = SCIPnodeGetLowerbound(node);
        // double upper = SCIPgetUpperbound(scip);
        double gap = SCIPcomputeGap(SCIPepsilon(scip), SCIPinfinity(scip), SCIPgetPrimalbound(scip), SCIPnodeGetLowerbound(node));
        
        // double gap = (upper - lower)/ lower;
        int node_id = SCIPnodeGetNumber(node);
        if (node_id >= node_rolling_lp_gap.size()) {
            auto new_size = node_id + 1;
            // std::cout<< "Array size: " << node_rolling_lp_gap.size() << ". Resizing to be " << new_size << std::endl;
            node_rolling_lp_gap.resize(new_size);
        }
        // std::cout<< "Pushing into rolling list..." << std::endl;
        pushIntoRollingLpGapList(node_rolling_lp_gap[node_id], gap, sec_max_tailing_off_iterations);
        // std::cout<< "LPSOLSTAT=" << SCIPgetLPSolstat(scip) << std::endl;
        if (isNodeTailingOff(node_rolling_lp_gap[node_id], sec_lp_gap_improvement_threshold, sec_max_tailing_off_iterations)
            && (SCIPgetLPSolstat(scip) == SCIP_LPSOLSTAT_UNBOUNDEDRAY || SCIPgetLPSolstat(scip) == SCIP_LPSOLSTAT_OPTIMAL)) {
            std::cout<< "BRANCHING: Node " << node_id << " found to be tailing off. Gap is " << gap << ". Threshold is " << sec_lp_gap_improvement_threshold << std::endl;
            // std::cout<< ". Num Branching candidates: " << SCIPgetNLPBranchCands(scip) << std::endl;
            // SCIPbranchLP(scip, result);
            *result = SCIP_BRANCHED;
        }
        else {
            BOOST_LOG_TRIVIAL(debug) << "SCIP enfolp: LP is not simple cycle";
            // *result = SCIP_FEASIBLE;
            // SCIP_CALL(PCTSPseparateSubtour(scip, conshdlr, conss, nconss, nusefulconss, NULL, result, sec_disjoint_tour, sec_maxflow_mincut));
            *result = SCIP_INFEASIBLE;
        }
    }
    SCIP_NODE* n = SCIPgetCurrentNode(scip);
    int nchildren;
    SCIP_NODE** children;
    SCIPgetChildren(scip, &children, &nchildren);
    // std::cout<< "N children of node " <<  SCIPnodeGetNumber(n) << ": " << nchildren << std::endl;
    return SCIP_OKAY;
}

SCIP_DECL_CONSTRANS(PCTSPconshdlrSubtour::scip_trans) {
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
        BOOST_LOG_TRIVIAL(debug) << message;
    }
    return SCIP_OKAY;
}

void pushIntoRollingLpGapList(std::list<double>& rolling_gaps, double& gap, int& sec_max_tailing_off_iterations) {
    if (sec_max_tailing_off_iterations >= 1) {
        // std::cout<< "Size of list: " << rolling_gaps.size() << std::endl;
        rolling_gaps.push_back(gap);
        if (rolling_gaps.size() > sec_max_tailing_off_iterations)
            rolling_gaps.pop_front();
    }
}

bool isNodeTailingOff(
    std::list<double>& rolling_gaps,
    double& sec_lp_gap_improvement_threshold,
    int& sec_max_tailing_off_iterations
) {
    if ((sec_max_tailing_off_iterations >= 1) && (rolling_gaps.size() >= 2)) {
        return (rolling_gaps.front() - rolling_gaps.back() < sec_lp_gap_improvement_threshold)
            && (rolling_gaps.size() == sec_max_tailing_off_iterations);
    }
    return false;
}


SCIP_DECL_CONSSEPALP(PCTSPconshdlrSubtour::scip_sepalp) {
    *result = SCIP_DIDNOTFIND;
    SCIP_CALL(PCTSPseparateSubtour(scip, conshdlr, conss, nconss, nusefulconss, NULL, result, sec_disjoint_tour, sec_maxflow_mincut));
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPASOL(PCTSPconshdlrSubtour::scip_sepasol) {
    SCIP_CALL(PCTSPseparateSubtour(scip, conshdlr, conss, nconss, nusefulconss, sol, result, sec_disjoint_tour, sec_maxflow_mincut));
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
    int& num_conss_added
) {
    typedef adjacency_list_traits< vecS, vecS, directedS > Traits;
    typedef boost::property< edge_reverse_t, Traits::edge_descriptor > ReverseEdges;
    typedef boost::property< edge_residual_capacity_t, CapacityType, ReverseEdges> ResidualCapacityMap;
    typedef boost::adjacency_list<
        boost::vecS,
        boost::vecS,
        boost::directedS,
        boost::no_property,
        property< edge_capacity_t, CapacityType, ResidualCapacityMap>> DirectedCapacityGraph;
    DirectedCapacityGraph support_graph;
    // create a capacity map for the edges
    // the capacity of each edge is the value in the solution given
    auto solution_edges = getSolutionEdges(scip, input_graph, sol, edge_variable_map);
    CapacityVector capacity_vector = getCapacityVectorFromSol(scip, input_graph, sol, edge_variable_map);

    // only get edges that are in the same connected components as the root
    VertexPairVector edge_vector = getVertexPairVectorFromEdgeSubset(input_graph, solution_edges);
    VertexPairVector root_edge_vector;

    for (auto pair : edge_vector) {
        if (root_component.count(pair.first) >= 1 && root_component.count(pair.second) >= 1) {
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
    std::vector<bool> added_sec(boost::num_vertices(support_graph));
    for (int i = 0; i < added_sec.size(); i++) {
        added_sec[i] = false;
    }
    if (support_root < added_sec.size())
        added_sec[support_root] = true;
    else
        BOOST_LOG_TRIVIAL(debug) << "Num vertices in support graph is zero.";
    for (auto target : boost::make_iterator_range(boost::vertices(support_graph))) {
        if (!added_sec[target]) {
            // reset the residual capacity
            for (auto edge : boost::make_iterator_range(boost::edges(support_graph))) {
                residual_capacity[edge] = 0;
            }
            auto flow = boost::push_relabel_max_flow(support_graph, support_root, target, capacity_property, residual_capacity, reverse_edges, vindex);

            if (flow < 2 * FLOW_FLOAT_MULTIPLIER) {
                // traverse the residual graph from the root
                // all vertices that are reachable on edges that have some flow are on one side of the cut
                // all edges that are not reachable by the flow are on the other side of the cut
                std::vector<PCTSPvertex> input_vertices;
                auto unreachable = getUnreachableVertices(support_graph, support_root, residual_capacity);
                if (unreachable.size() >= 3) {   // do not add SEC for small groups of vertices
                    // the component not containing the root violates the subtour elimination constraint
                    BOOST_LOG_TRIVIAL(debug) << unreachable.size() << " vertices are unreachable from root of the residual graph.";
                    input_vertices = getOldVertices(lookup, unreachable);
                }
                else {  // unreachable size is less than 3
                    auto reachable = getReachableVertices(support_graph, support_root, residual_capacity);
                    input_vertices = getOldVertices(lookup, reachable);
                }
                for (auto &unreachable_vertex: unreachable) {
                    // for each unreachable vertex add a subtour elimination constraint
                    auto input_target_vertex = getOldVertex(lookup, unreachable_vertex);
                    SCIP_CALL(addSubtourEliminationConstraint(
                        scip,
                        conshdlr,
                        input_graph,
                        input_vertices,
                        edge_variable_map,
                        root_vertex,
                        input_target_vertex,
                        sol,
                        result
                    ));
                    num_conss_added ++;
                    // mark the unreachable target vertex to remember we have already added a SEC
                    added_sec[getNewVertex(lookup, input_target_vertex)] = true;
                }
            }
        }
    }
    return SCIP_OKAY;
}

SCIP_RETCODE PCTSPseparateSubtour(
    SCIP* scip,               /**< SCIP data structure */
    SCIP_CONSHDLR* conshdlr,           /**< the constraint handler itself */
    SCIP_CONS** conss,              /**< array of constraints to process */
    int nconss,             /**< number of constraints to process */
    int nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
    SCIP_SOL* sol,                /**< primal solution that should be separated */
    SCIP_RESULT* result,              /**< pointer to store the result of the separation call */
    bool sec_disjoint_tour,
    bool sec_maxflow_mincut
) {
    // load the constraint handler data
    ProbDataPCTSP* probdata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    auto& input_graph = *(probdata->getInputGraph());
    auto& edge_variable_map = *(probdata->getEdgeVariableMap());
    auto& root_vertex = *(probdata->getRootVertex());

    // connected components
    auto support_graph = filterGraphByPositiveEdgeVars(scip, input_graph, sol, edge_variable_map);
    std::vector< int > component(boost::num_vertices(support_graph));
    int n_components = boost::connected_components(support_graph, &component[0]);
    auto component_vectors = getConnectedComponentsVectors(support_graph, n_components, component);
    auto v_index = boost::get(vertex_index, support_graph);
    int root_component_id = component[v_index[root_vertex]];

    bool node_eventhdlr_ready = false;
    auto objeventhdlr = SCIPfindObjEventhdlr(scip, NODE_EVENTHDLR_NAME.c_str());
    NodeEventhdlr* node_eventhdlr;
    if (objeventhdlr != 0) {
        node_eventhdlr =  dynamic_cast<NodeEventhdlr*>(objeventhdlr);
        node_eventhdlr_ready = true;
    }

    if (sec_disjoint_tour) {

        int num_disjoint_tour_secs_added = 0;
        PCTSPseparateDisjointTour(
            scip, conshdlr, input_graph, edge_variable_map, root_vertex, component_vectors, sol, result, root_component_id, num_disjoint_tour_secs_added
        );
        if (node_eventhdlr_ready) node_eventhdlr->incrementNumSecDisjointTour(scip, num_disjoint_tour_secs_added);
    }
    if (sec_maxflow_mincut)
    {
        // create a set from the root component vector
        std::set<PCTSPvertex> root_component;
        std::copy(component_vectors[root_component_id].begin(), component_vectors[root_component_id].end(), std::inserter(root_component, root_component.begin()));
    
        // separate SEC using maxflow mincut
        int num_maxflow_mincut_secs_added = 0;
        PCTSPseparateMaxflowMincut(
            scip, conshdlr, input_graph, edge_variable_map, root_vertex, sol, result, root_component, num_maxflow_mincut_secs_added
        );
        if (node_eventhdlr_ready) node_eventhdlr->incrementNumSecMaxflowMincut(scip, num_maxflow_mincut_secs_added);
    }
    return SCIP_OKAY;
}
