#ifndef __PCTSP_CYCLE_COVER__
#define __PCTSP_CYCLE_COVER__

#include <boost/graph/connected_components.hpp>
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

#include "data_structures.hh"
#include "graph.hh"
#include "sciputils.hh"
#include "solution.hh"

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

const std::string CYCLE_COVER_CONS_PREFIX = "cycle_cover_";

SCIP_RETCODE createCycleCoverCons(
    SCIP* scip,
    SCIP_CONS** cons,
    const std::string& name,
    SCIP_Bool             initial,            /**< should the LP relaxation of constraint be in the initial LP? */
    SCIP_Bool             separate,           /**< should the constraint be separated during LP processing? */
    SCIP_Bool             enforce,            /**< should the constraint be enforced during node processing? */
    SCIP_Bool             check,              /**< should the constraint be checked for feasibility? */
    SCIP_Bool             propagate,          /**< should the constraint be propagated during node processing? */
    SCIP_Bool             local,              /**< is constraint only valid locally? */
    SCIP_Bool             modifiable,         /**< is constraint modifiable (subject to column generation)? */
    SCIP_Bool             dynamic,            /**< is constraint dynamic? */
    SCIP_Bool             removable           /**< should the constraint be removed from the LP due to aging or cleanup? */
);

SCIP_RETCODE createBasicCycleCoverCons(SCIP* scip, SCIP_CONS** cons);

SCIP_RETCODE createBasicCycleCoverCons(SCIP* scip, SCIP_CONS** cons, const std::string& name);

template <typename TGraph, typename TEdgeVarMap>
std::vector<typename TGraph::vertex_descriptor> getRootComponent(
    SCIP* scip,
    TGraph& graph,
    typename TGraph::vertex_descriptor& root_vertex,
    TEdgeVarMap& edge_var_map,
    SCIP_SOL* sol
) {
    // get the connected component of the root
    auto support_graph = filterGraphByPositiveEdgeVars(scip, graph, sol, edge_var_map);
    std::vector< int > component(boost::num_vertices(support_graph));
    int n_components = boost::connected_components(support_graph, &component[0]);
    auto component_vectors = getConnectedComponentsVectors(support_graph, n_components, component);
    auto v_index = boost::get(vertex_index, support_graph);
    int root_component_id = component[v_index[root_vertex]];
    auto root_component = component_vectors[root_component_id];
    return root_component;
}

template <typename TGraph, typename TPrizeMap, typename TPrizeType, typename TEdgeVarMap>
bool isCycleCoverViolated(
    SCIP* scip,
    TGraph& graph,
    TPrizeMap& prize_map,
    TPrizeType& quota,
    typename TGraph::vertex_descriptor& root_vertex,
    TEdgeVarMap& edge_var_map,
    SCIP_SOL* sol
) {
    auto root_component = getRootComponent(scip, graph, root_vertex, edge_var_map, sol);

    // test if the prize of the root component is above the quota
    return ! isPrizeFeasible(prize_map, quota, root_component);
}

bool isCycleCoverViolated(SCIP* scip, SCIP_SOL* sol, ProbDataPCTSP* probdata);

SCIP_RETCODE separateCycleCover(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_SOL* sol, SCIP_RESULT* result);

template <typename TGraph, typename TPrizeMap, typename TPrize, typename TEdgeVarMap>
SCIP_RETCODE separateCycleCover(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    SCIP_SOL* sol,
    SCIP_RESULT* result,
    TGraph& graph,
    TPrizeMap& prize_map,
    TPrize& quota,
    typename TGraph::vertex_descriptor& root_vertex,
    TEdgeVarMap& edge_var_map
) {
    auto root_component = getRootComponent(scip, graph, root_vertex, edge_var_map, sol);
    if (! isPrizeFeasible(prize_map, quota, root_component)) {
        auto first = root_component.begin();
        auto last = root_component.end();
        addCycleCover(scip, conshdlr, graph, first, last, edge_var_map, sol, result);
    }
    return SCIP_OKAY;
}


const std::string CYCLE_COVER_NAME = "Cycle cover";
const std::string CYCLE_COVER_DESCRIPTION = "Inequalities are added if a set of vertices cannot contain a feasible cycle";

class CycleCoverConshdlr : public scip::ObjConshdlr {
private:
    int _num_conss_added;

public:
    CycleCoverConshdlr(SCIP* scip, const std::string& name, const std::string& description)
        : ObjConshdlr(scip, name.c_str(), description.c_str(), 1000000, -2000000, -2000000, 1, -1, 1, 0,
            FALSE, FALSE, TRUE, SCIP_PROPTIMING_BEFORELP, SCIP_PRESOLTIMING_FAST)
    { 
        _num_conss_added = 0;
    }

    CycleCoverConshdlr(SCIP* scip)
        : CycleCoverConshdlr(scip, CYCLE_COVER_NAME.c_str(), CYCLE_COVER_DESCRIPTION.c_str())
    {
        _num_conss_added = 0;
    }

    int getNumConssAdded();
    SCIP_DECL_CONSCHECK(scip_check);
    SCIP_DECL_CONSENFOPS(scip_enfops);
    SCIP_DECL_CONSENFOLP(scip_enfolp);
    SCIP_DECL_CONSTRANS(scip_trans);
    SCIP_DECL_CONSLOCK(scip_lock);
    SCIP_DECL_CONSSEPALP(scip_sepalp);
};

#endif