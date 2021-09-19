#ifndef __PCTSP_CYCLE_COVER__
#define __PCTSP_CYCLE_COVER__

#include <boost/graph/graph_traits.hpp>
#include <objscip/objscip.h>

#include "graph.hh"
#include "sciputils.hh"

template <typename TGraph, typename TVertexIt, typename TEdgeVarMap>
SCIP_RETCODE addCycleCover(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    TGraph& graph,
    TVertexIt& first_vertex_it,
    TVertexIt& last_vertex_it,
    TEdgeVarMap& edge_variable_map,
    SCIP_SOL* sol,
    SCIP_RESULT* result
) {
    // do nothing if there are no vertices to iterate over
    if (std::distance(first_vertex_it, last_vertex_it) == 0) return SCIP_OKAY;

    // get induced edge variables
    auto first_vertex_it_copy1 = first_vertex_it;    // make a copy of the start iterator
    auto induced_edges = getEdgesInducedByVertices(graph, first_vertex_it_copy1, last_vertex_it);
    auto edge_var_vector = getEdgeVariables(scip, graph, edge_variable_map, induced_edges);
    assert (first_vertex_it != first_vertex_it_copy1);

    // get vertex variables
    auto first_vertex_it_copy2 = first_vertex_it;    // make a copy of the start iterator
    auto self_loops = getSelfLoops(graph, first_vertex_it_copy2, last_vertex_it);
    auto vertex_var_vector = getEdgeVariables(scip, graph, edge_variable_map, self_loops);
    assert (first_vertex_it != first_vertex_it_copy2);

    // x(E(S)) <= y(S) - 1
    auto nvars = edge_var_vector.size() + vertex_var_vector.size();
    VarVector all_vars(nvars);
    std::vector<double> var_coefs(nvars);
    fillPositiveNegativeVars(edge_var_vector, vertex_var_vector, all_vars, var_coefs);
    double lhs = -SCIPinfinity(scip);
    double rhs = -1;
    std::string name = "CycleCover_" + joinVariableNames(all_vars);

    // add constraint/row
    return addRow(scip, conshdlr, result, sol, all_vars, var_coefs, lhs, rhs, name);
}

SCIP_RETCODE addCycleCover(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    PCTSPgraph& graph,
    std::vector<PCTSPvertex>& vertices_in_cover,
    PCTSPedgeVariableMap& edge_variable_map,
    SCIP_SOL* sol,
    SCIP_RESULT* result
);

#endif