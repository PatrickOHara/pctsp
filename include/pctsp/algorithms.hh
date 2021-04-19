#ifndef __PCTSP_PCTSP__
#define __PCTSP_PCTSP__
/** Exact algorithms for PCTSP */

#include <iostream>
#include "scip/message_default.h"

#include "constraint.hh"
#include "logger.hh"
#include "preprocessing.hh"
using namespace boost;
using namespace scip;
using namespace std;

template <typename Edge>
const char*
getVariableNameFromEdge(std::map<Edge, SCIP_VAR*>& edge_variable_map,
    Edge edge) {
    SCIP_VAR* variable = edge_variable_map[edge];
    const char* variable_name = SCIPvarGetName(variable);
    return variable_name;
}

/** Add edge variables to the PCTSP SCIP model */
template <typename Graph, typename CostMap, typename VariableMap>
SCIP_RETCODE PCTSPaddEdgeVariables(SCIP* mip, Graph& graph, CostMap& cost_map,
    VariableMap& variable_map) {
    for (auto edge : make_iterator_range(edges(graph))) {
        SCIP_VAR* edge_variable;
        int cost_of_edge = cost_map[edge];
        SCIP_CALL(SCIPcreateVar(mip, &edge_variable, NULL, 0.0, 1.0,
            cost_of_edge, SCIP_VARTYPE_BINARY, TRUE, FALSE,
            NULL, NULL, NULL, NULL, NULL));

        SCIP_CALL(SCIPaddVar(mip, edge_variable));
        variable_map[edge] = edge_variable;
        // release variable after adding it to model
        // SCIP_CALL(SCIPreleaseVar(mip, &edge_variable));
    }
    return SCIP_OKAY;
}

/** Get a SCIP model of the prize-collecting TSP without any subtour
 * elimiation constraints (SECs).
 *
 * This function sets the objective function, adds the variables,
 * and adds the constraints.
 */
template <typename Graph, typename Vertex, typename CostMap, typename WeightMap>
SCIP_RETCODE PCTSPmodelWithoutSECs(
    SCIP* mip, Graph& graph, CostMap& cost_map, WeightMap& weight_map,
    int quota, Vertex root_vertex,
    std::map<typename Graph::edge_descriptor, SCIP_VAR*>& variable_map) {
    // from the graph, create the variables on edges and nodes
    SCIP_CALL(PCTSPaddEdgeVariables(mip, graph, cost_map, variable_map));
    int nvars = SCIPgetNVars(mip);

    // add objective function
    SCIP_CALL(SCIPsetObjsense(mip, SCIP_OBJSENSE_MINIMIZE));
    // add the prize constraint to the solver
    SCIP_CALL(
        PCTSPaddPrizeConstraint(mip, variable_map, weight_map, quota, nvars));

    // add constraint that the root vertex must be visited
    auto root_vd = vertex(root_vertex, graph); // get vertex descriptor
    auto root_self_loop = edge(root_vd, root_vd, graph);
    if (root_self_loop.second == false) {
        throw EdgeNotFoundException(std::to_string(root_vd),
            std::to_string(root_vd));
    }
    SCIP_CALL(
        PCTSPaddRootVertexConstraint(mip, variable_map, root_self_loop.first));

    // add constraint to ensure a vertex is adjacent to exactly two edges in
    // the tour
    SCIP_CALL(PCTSPaddDegreeTwoConstraint(mip, graph, variable_map));

    return SCIP_OKAY;
}

template <typename Edge>
SCIP_RETCODE
PCTSPgetEdgeListFromSolution(SCIP* mip, SCIP_SOL* sol,
    std::map<Edge, SCIP_VAR*>& edge_variable_map,
    std::list<Edge>& edge_list) {
    // TODO iterate over edge variable map
    // get edges where the value of the variable are equal to one

    for (auto const& [edge, variable] : edge_variable_map) {
        double var_value = SCIPgetSolVal(mip, sol, variable);
        if (var_value == 1) {
            edge_list.push_back(edge);
        }
    }
    return SCIP_OKAY;
}

/** Solve the Prize Collecting TSP problem using a branch and cut algorithm
 */
template <typename Graph, typename Vertex, typename Edge, typename CostMap,
    typename PrizeMap>
    SCIP_RETCODE PCTSPbranchAndCut(Graph& graph, std::list<Edge>& optimal_edge_list,
        CostMap& cost_map, PrizeMap& prize_map,
        int quota, Vertex root_vertex, const char* log_filepath = NULL, bool print_scip = true) {

    // initialise empty model
    SCIP* mip = NULL;
    SCIP_CALL(SCIPcreate(&mip));
    SCIP_CALL(SCIPincludeDefaultPlugins(mip));
    SCIP_CALL(SCIPcreateProbBasic(mip, "pctsp"));
    SCIP_MESSAGEHDLR* handler;
    SCIP_CALL(SCIPcreateMessagehdlrDefault(&handler, false, log_filepath, print_scip));
    SCIP_CALL(SCIPsetMessagehdlr(mip, handler));

    BOOST_LOG_TRIVIAL(info) << "Created SCIP program. Adding constraints and variables.";

    // datastructures needed for the MIP solver
    std::map<Edge, SCIP_VAR*> edge_variable_map;
    std::map<Edge, int> weight_map;

    // move prizes of vertices onto the weight of an edge
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // add variables and constraints to SCIP model
    SCIP_CALL(PCTSPmodelWithoutSECs(mip, graph, cost_map, weight_map, quota,
        root_vertex, edge_variable_map));
    int nvars = SCIPgetNVars(mip);

    // TODO add the subtour elimination constraints as cutting planes

    // TODO add the cost cover inequalities as cutting planes

    // TODO add selected heuristics to reduce the upper bound on the optimal

    // TODO adjust parameters for the branching strategy

    BOOST_LOG_TRIVIAL(info) << "Added contraints and variables. Solving model.";
    // Solve the model
    SCIP_CALL(SCIPsolve(mip));
    BOOST_LOG_TRIVIAL(info) << "Model solved. Getting edge list of best solution.";
    // Get the solution
    SCIP_SOL* sol = SCIPgetBestSol(mip);
    PCTSPgetEdgeListFromSolution(mip, sol, edge_variable_map, optimal_edge_list);
    if (print_scip) {
        BOOST_LOG_TRIVIAL(debug) << "Saving SCIP logs to: " << log_filepath;
        FILE* log_file = fopen(log_filepath, "w");
        SCIP_CALL(SCIPprintOrigProblem(mip, log_file, NULL, true));
        SCIP_CALL(SCIPprintBestSol(mip, log_file, true));
    }

    BOOST_LOG_TRIVIAL(info) << "Releasing SCIP model.";
    BOOST_LOG_TRIVIAL(debug) << "Releasing constraint handler.";
    SCIP_CALL(SCIPmessagehdlrRelease(&handler));

    BOOST_LOG_TRIVIAL(debug) << "Releasing the model itself.";
    SCIP_CALL(SCIPfree(&mip));
    BOOST_LOG_TRIVIAL(debug) << "Done releasing model. Returning status SCIP_OKAY.";
    // from the variables, obtain and optimal tour
    return SCIP_OKAY;
}
#endif