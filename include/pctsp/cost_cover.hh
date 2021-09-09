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
   }

   /** destructor of event handler to free user data (called when SCIP is exiting) */
   virtual SCIP_DECL_EVENTFREE(scip_free);

   /** initialization method of event handler (called after problem was transformed) */
   virtual SCIP_DECL_EVENTINIT(scip_init);

   /** deinitialization method of event handler (called before transformed problem is freed) */
   virtual SCIP_DECL_EVENTEXIT(scip_exit);

   /** solving process initialization method of event handler (called when branch and bound process is about to begin)
    *
    *  This method is called when the presolving was finished and the branch and bound process is about to begin.
    *  The event handler may use this call to initialize its branch and bound specific data.
    *
    */
   virtual SCIP_DECL_EVENTINITSOL(scip_initsol);

   /** solving process deinitialization method of event handler (called before branch and bound process data is freed)
    *
    *  This method is called before the branch and bound process is freed.
    *  The event handler should use this call to clean up its branch and bound data.
    */
   virtual SCIP_DECL_EVENTEXITSOL(scip_exitsol);

   /** frees specific constraint data */
   virtual SCIP_DECL_EVENTDELETE(scip_delete);

   /** execution method of event handler
    *
    *  Processes the event. The method is called every time an event occurs, for which the event handler
    *  is responsible. Event handlers may declare themselves resposible for events by calling the
    *  corresponding SCIPcatch...() method. This method creates an event filter object to point to the
    *  given event handler and event data.
    */
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
    return includeShortestPathCostCover(scip, distances);
}

#endif