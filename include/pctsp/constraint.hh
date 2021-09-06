#ifndef __PCTSP_CONSTRAINT__
#define __PCTSP_CONSTRAINT__

/** Constraints for the Prize Collecting TSP
 *
 */

#include <boost/graph/adjacency_list.hpp>
#include <map>
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

#include "exception.hh"

using namespace boost;
using namespace scip;
using namespace std;

template <typename Edge>
std::map<const char *, int>
getVariableNameToWeightMap(std::map<Edge, SCIP_VAR *> &edge_variable_map,
                           std::map<Edge, int> weight_map) {
    std::map<const char *, int> variable_name_to_weight_map;
    for (auto it = edge_variable_map.begin(); it != edge_variable_map.end();
         it++) {
        auto edge = it->first;
        SCIP_VAR *variable = it->second;
        const char *variable_name = SCIPvarGetName(variable);
        variable_name_to_weight_map[variable_name] = weight_map[edge];
    }
    return variable_name_to_weight_map;
}

template <typename Edge, typename EdgeVariableMap>
SCIP_RETCODE PCTSPaddRootVertexConstraint(SCIP *scip,
                                          EdgeVariableMap edge_variable_map,
                                          Edge root_self_loop) {
    SCIP_VAR *root_variable = edge_variable_map[root_self_loop];
    SCIP_CONS *cons = nullptr;
    SCIP_VAR *vars[1];
    vars[0] = root_variable;
    double values[1];
    values[0] = 1;
    SCIP_CALL(SCIPcreateConsBasicLinear(scip, &cons, "root-constraint", 1, vars,
                                        values, 1, 1));
    SCIP_CALL(SCIPaddCons(scip, cons));
    SCIP_CALL(SCIPreleaseCons(scip, &cons));
    return SCIP_OKAY;
}

template <typename Graph, typename EdgeVariableMap>
SCIP_RETCODE PCTSPaddDegreeTwoConstraint(SCIP *scip, Graph &graph,
                                         EdgeVariableMap &edge_variable_map) {
    // for each vertex in the graph, get the variable that represents the vertex
    // then add a constraint that sets the sum of the variables of the neighbors
    // to be equal to the value of the vertex variable.
    int cons_count = 0;
    for (auto vertex : boost::make_iterator_range(vertices(graph))) {
        // get the edge that represents a self loop on the vertex
        auto e = edge(vertex, vertex, graph);

        // get the variable of the self loop
        if (e.second == false) {
            std::string str_vertex = to_string(vertex);
            throw EdgeNotFoundException(str_vertex, str_vertex);
        }
        SCIP_VAR *variable = edge_variable_map[e.first];

        // get the variables of the neighbors
        int neighbor_index = 1;

        // self loops have degree 2
        int num_neighbors = degree(vertex, graph) - 1;

        SCIP_CONS *cons = nullptr;
        std::string cons_name =
            std::string("degree-two-constraint-") + std::to_string(cons_count);
        SCIP_VAR *vars[num_neighbors];
        double coefs[num_neighbors];
        vars[0] = variable;
        coefs[0] = -2.0;
        for (auto neighbor :
             make_iterator_range(adjacent_vertices(vertex, graph))) {
            auto adjacent_edge = edge(vertex, neighbor, graph);
            if (adjacent_edge.first != e.first) {
                SCIP_VAR *neighbor_var = edge_variable_map[adjacent_edge.first];
                vars[neighbor_index] = neighbor_var;
                coefs[neighbor_index] = 1;
                neighbor_index++;
            }
        }
        // add the variables to an equality constraint
        SCIP_CALL(SCIPcreateConsBasicLinear(scip, &cons, cons_name.c_str(),
                                            num_neighbors, vars, coefs, 0, 0));
        SCIP_CALL(SCIPaddCons(scip, cons));
        // release the constraint
        SCIP_CALL(SCIPreleaseCons(scip, &cons));

        cons_count++;
    }

    return SCIP_OKAY;
}

/** Add a knapsack inequality to ensure the collected prize (weight)
 * is at least the quota.
 */
template <typename VariableMap, typename WeightMap>
SCIP_RETCODE PCTSPaddPrizeConstraint(SCIP *scip, VariableMap &variable_map,
                                     WeightMap &weight_map, int quota,
                                     int num_edge_variables) {
    SCIP_CONS *cons = nullptr;

    // arrays to store variables and weights of items in knapsack
    SCIP_VAR **vars = SCIPgetVars(scip);

    std::map<const char *, int> variable_name_to_weight_map =
        getVariableNameToWeightMap(variable_map, weight_map);

    // take the negative quota and set it to be the capacity of the knapsack
    // SCIP_CALL(SCIPcreateConsBasicLinear(scip, &cons, "prize-constraint", 0,
    // NULL,
    //                                     NULL, -SCIPinfinity(scip), -quota));
    SCIP_CALL(SCIPcreateConsLinear(scip, &cons, "prize-constraint", 0, NULL,
                                   NULL, -SCIPinfinity(scip), -quota, TRUE, TRUE,
                                   TRUE, TRUE, TRUE, FALSE, false, FALSE, FALSE,
                                   FALSE));
    for (int i = 0; i < num_edge_variables; i++) {
        SCIP_VAR *variable = vars[i];
        int weight = -variable_name_to_weight_map[SCIPvarGetName(variable)];
        SCIP_CALL(SCIPaddCoefLinear(scip, cons, variable, weight));
        SCIP_CALL(SCIPreleaseVar(scip, &variable));
    }

    SCIP_CALL(SCIPaddCons(scip, cons));

    // remember to release the constraint once done with it
    SCIP_CALL(SCIPreleaseCons(scip, &cons));

    return SCIP_OKAY;
}

#endif