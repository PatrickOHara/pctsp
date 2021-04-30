
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

SCIP_RETCODE addSubtourEliminationConstraint(SCIP* mip, PCTSPgraph& graph, std::vector<PCTSPvertex>& vertex_set, PCTSPedgeVariableMap& edge_variable_map, PCTSPvertex& root_vertex, PCTSPvertex& target_vertex) {

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
    SCIP_CALL(SCIPcreateConsBasicLinear(mip, &cons, cons_name.c_str(), nvars, vars, vals, lhs, rhs));

    // add the constraint to the solver
    SCIP_CALL(SCIPaddCons(mip, cons));
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