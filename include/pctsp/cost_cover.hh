#ifndef __PCTSP_COST_COVER__
#define __PCTSP_COST_COVER__

#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

#include "pctsp/exception.hh"
#include "pctsp/graph.hh"

/**
 * @brief Find vertices that cannot be reached from the source
 * in cost less than or equal to the cost upper bound.
 * 
 * @tparam UndirectedGraph 
 * @tparam CostMap 
 * @param graph 
 * @param cost_map 
 * @param source_vertex 
 * @param cost_upper_bound 
 * @return std::vector<boost::graph_traits<Graph>::vertex_descriptor> 
 */
template <class Vertex>
std::vector<Vertex> separateCostCoverPaths(
    std::map<Vertex, int>& cost_map,
    int cost_upper_bound
) {
    std::vector<Vertex> separated_vertices;
    for (auto const& [key, val] : cost_map){
        if (val > cost_upper_bound) {
            separated_vertices.push_back(key);
        }
    }
    return separated_vertices;
}

SCIP_RETCODE addCoverInequality(
    SCIP* scip,
    std::vector<SCIP_VAR*>& variables,
    SCIP_RESULT* result,
    SCIP_SOL* sol
);

template <class UndirectedGraph>
SCIP_RETCODE addCoverInequalityFromVertices(
    SCIP* scip,
    UndirectedGraph& graph,
    std::vector<typename boost::graph_traits<UndirectedGraph>::vertex_descriptor>& cover_vertices,
    std::map<typename boost::graph_traits<UndirectedGraph>::edge_descriptor, SCIP_VAR*>& edge_variable_map,
    SCIP_RESULT* result,
    SCIP_SOL* sol
) {
    // get edges of the self loops
    auto vertices_start = cover_vertices.begin();
    auto vertices_last = cover_vertices.end();
    auto self_loop_edges = getSelfLoops(graph, vertices_start, vertices_last);

    // get variables of the self loops
    auto edges_start = self_loop_edges.begin();
    auto edges_last = self_loop_edges.end();
    auto vars = getEdgeVariables(scip, graph, edge_variable_map, edges_start, edges_last);

    // add a cover inequality over the variables
    SCIP_CALL(addCoverInequality(scip, vars, result, sol));
    return SCIP_OKAY;
}

class CostCoverConshdlr : public scip::ObjConshdlr
{
private:
    bool cost_cover_disjoint_paths;
    bool cost_cover_shortest_path;
    bool cost_cover_steiner_tree;
    
public:

    CostCoverConshdlr(
        SCIP* scip,
        bool _cost_cover_disjoint_paths,
        bool _cost_cover_shortest_path,
        bool _cost_cover_steiner_tree
    ) : ObjConshdlr(scip, "cost_cover", "PCTSP cost cover constraints",
            1000000, -2000000, -2000000, 1, -1, 1, 0,
            FALSE, FALSE, TRUE, SCIP_PROPTIMING_BEFORELP, SCIP_PRESOLTIMING_FAST)
    {
        cost_cover_disjoint_paths = _cost_cover_disjoint_paths;
        cost_cover_shortest_path = _cost_cover_shortest_path;
        cost_cover_steiner_tree = _cost_cover_steiner_tree;
    }

    SCIP_DECL_CONSCHECK(scip_check);
    SCIP_DECL_CONSENFOPS(scip_enfops);
    SCIP_DECL_CONSENFOLP(scip_enfolp);
    SCIP_DECL_CONSTRANS(scip_trans);
    SCIP_DECL_CONSLOCK(scip_lock);
    SCIP_DECL_CONSPRINT(scip_print);
    SCIP_DECL_CONSSEPALP(scip_sepalp);
    SCIP_DECL_CONSSEPASOL(scip_sepasol);
};


#endif