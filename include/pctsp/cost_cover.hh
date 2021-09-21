#ifndef __PCTSP_COST_COVER__
#define __PCTSP_COST_COVER__

#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>

#include "pctsp/exception.hh"
#include "pctsp/graph.hh"

const std::string COST_COVER_CONS_PREFIX = "cost_cover_";

/**
 * @brief Find vertices that cannot be reached from the source
 * in cost less than or equal to the cost upper bound.
 */
template <typename TGraph>
std::vector<typename boost::graph_traits<TGraph>::vertex_descriptor> separateCostCoverPaths(
    TGraph& graph,
    std::vector<int>& path_distances,
    int cost_upper_bound
) {
    typedef typename boost::graph_traits<TGraph>::vertex_descriptor Vertex;
    std::vector<Vertex> separated_vertices;
    for (auto vertex: boost::make_iterator_range(boost::vertices(graph))) {
        if (path_distances[vertex] > cost_upper_bound) {
            separated_vertices.push_back(vertex);
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

template <typename TGraph>
SCIP_RETCODE addCoverInequalityFromVertices(
    SCIP* scip,
    TGraph& graph,
    std::vector<typename boost::graph_traits<TGraph>::vertex_descriptor>& cover_vertices,
    std::map<typename boost::graph_traits<TGraph>::edge_descriptor, SCIP_VAR*>& edge_variable_map,
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


class CostCoverEventHandler : public scip::ObjEventhdlr
{

private:
    std::vector<int> _path_distances;
    int _num_conss_added;

public:
    CostCoverEventHandler(SCIP* scip, const std::string& name, const std::string& description)
        : ObjEventhdlr(scip, name.c_str(), description.c_str())
    {

    }

   CostCoverEventHandler(
        SCIP* scip,
        const std::string& name,
        const std::string& description,
        std::vector<int>& path_distances
    )
        : ObjEventhdlr(scip, name.c_str(), description.c_str())
    {
        _path_distances = path_distances;
        _num_conss_added = 0;
    }
    int getNumConssAdded();
    std::vector<int> getPathDistances();
    virtual SCIP_DECL_EVENTFREE(scip_free);
    virtual SCIP_DECL_EVENTINIT(scip_init);
    virtual SCIP_DECL_EVENTEXIT(scip_exit);
    virtual SCIP_DECL_EVENTINITSOL(scip_initsol);
    virtual SCIP_DECL_EVENTEXITSOL(scip_exitsol);
    virtual SCIP_DECL_EVENTDELETE(scip_delete);
    virtual SCIP_DECL_EVENTEXEC(scip_exec);
};

const std::string SHORTEST_PATH_COST_COVER_NAME = "Shortest path cost cover";
const std::string DISJOINT_PATHS_COST_COVER_NAME = "Disjoint paths cost cover";
const std::string COST_COVER_DESCRIPTION = "Cost cover inequality event handlers are triggered when a new best solution is found.";

SCIP_RETCODE includeCostCoverEventHandler(
    SCIP* scip,
    const std::string& name,
    const std::string& description,
    std::vector<int>& path_distances
);

SCIP_RETCODE includeShortestPathCostCover(SCIP* scip, std::vector<int>& path_distances);

SCIP_RETCODE includeDisjointPathsCostCover(SCIP* scip, std::vector<int>& path_distances);

template <typename TGraph, typename TCostMap>
SCIP_RETCODE includeShortestPathCostCover(
    SCIP* scip,
    TGraph& graph,
    TCostMap& cost_map,
    typename boost::graph_traits<TGraph>::vertex_descriptor& source_vertex
) {
    typedef typename boost::graph_traits<TGraph>::vertex_descriptor Vertex;
    std::vector<Vertex> pred (boost::num_vertices(graph));
    std::vector<int> distances (boost::num_vertices(graph));
    auto vindex = get(boost::vertex_index, graph);
    auto pred_map = boost::predecessor_map(boost::make_iterator_property_map(pred.begin(), vindex));
    dijkstra_shortest_paths(
        graph,
        source_vertex,
        pred_map.distance_map(boost::make_iterator_property_map(
            distances.begin(), vindex
        ))
    );
    // multiply each of the distances by two:
    // when visiting a vertex, we must return to the root vertex
    for (int i = 0; i < distances.size(); i++) {
        distances[i] = distances[i] * 2;
    }
    return includeShortestPathCostCover(scip, distances);
}

#endif