#ifndef __PCTSP_SUBTOUR_ELIMINATION__
#define __PCTSP_SUBTOUR_ELIMINATION__

#include "data_structures.hh"
#include "solution.hh"
#include "graph.hh"
#include "renaming.hh"
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <objscip/objscip.h>

template <class UndirectedGraph, class ParityMap>
std::vector<typename boost::graph_traits<UndirectedGraph>::edge_descriptor> getEdgesFromCut(UndirectedGraph& graph, ParityMap& parity_map) {
    typedef typename boost::graph_traits<UndirectedGraph>::edge_descriptor Edge;
    std::vector<Edge> edges;
    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        // get edges where the endpoints lie in different cut sets
        auto source = boost::source(edge, graph);
        auto target = boost::target(edge, graph);
        if (boost::get(parity_map, source) != boost::get(parity_map, target)) {
            edges.push_back(edge);
        }
    }
    return edges;
}

typedef typename std::vector<SCIP_VAR*> VarVector;

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
);

SCIP_RETCODE PCTSPseparateSubtour(
    SCIP* scip,                 /**< SCIP data structure */
    SCIP_CONSHDLR* conshdlr,    /**< the constraint handler itself */
    SCIP_CONS** conss,              /**< array of constraints to process */
    int nconss,             /**< number of constraints to process */
    int nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
    SCIP_SOL* sol,              /**< primal solution that should be separated */
    SCIP_RESULT* result,         /**< pointer to store the result of the separation call */
    bool sec_disjoint_tour,
    int sec_disjoint_tour_freq,
    bool sec_maxflow_mincut,
    int sec_maxflow_mincut_freq
);

template <typename EdgeWeightMap>
struct positive_edge_weight {
    positive_edge_weight() { }
    positive_edge_weight(EdgeWeightMap weight) : m_weight(weight) { }
    template <typename Edge>
    bool operator()(const Edge& e) const {
        return 0 < get(m_weight, e);
    }
    EdgeWeightMap m_weight;
};

template<typename Graph, typename Weight>
std::vector<typename boost::graph_traits<Graph>::vertex_descriptor> getUnreachableVertices(
    Graph& graph,
    typename boost::graph_traits<Graph>::vertex_descriptor& source_vertex,
    Weight& weight
) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    std::vector<Vertex> unreachable_vertices;

    // filter the graph to get edges that have positive weight
    positive_edge_weight<Weight> filter(weight);
    boost::filtered_graph<Graph, positive_edge_weight<Weight> > f_graph(graph, filter);

    // create a colour map
    auto indexmap = boost::get(boost::vertex_index, f_graph);
    auto colormap = boost::make_vector_property_map<boost::default_color_type>(indexmap);

    // use DFS search to assign colours to vertices
    boost::default_dfs_visitor visitor;
    boost::depth_first_visit(f_graph, source_vertex, visitor, colormap);

    // get the colour of the source vertex
    auto source_color = boost::get(colormap, source_vertex);

    // return all vertices with a different colour to the source vertex
    for (auto vertex : boost::make_iterator_range(boost::vertices(f_graph))) {
        if (colormap[vertex] != source_color) {
            unreachable_vertices.push_back(vertex);
        }
    }
    return unreachable_vertices;
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
    int& num_conss_added,
    int freq
);

SCIP_RETCODE PCTSPseparateMaxflowMincut(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    PCTSPgraph& input_graph,
    PCTSPedgeVariableMap& edge_variable_map,
    PCTSPvertex& root_vertex,
    SCIP_SOL* sol,
    SCIP_RESULT* result,
    std::set<PCTSPvertex>& root_component,
    int& num_conss_added,
    int freq
);

void insertEdgeVertexVariables(VarVector& edge_variables,
    VarVector& vertex_variables,
    VarVector& all_variables,
    std::vector<double>& var_coefs
);

std::vector<PCTSPedge> getInducedEdges(PCTSPgraph& graph, std::vector<PCTSPvertex>& vertices);


class PCTSPconshdlrSubtour : public scip::ObjConshdlr
{

private:

    bool sec_disjoint_tour;
    int sec_disjoint_tour_freq;
    bool sec_maxflow_mincut;
    int sec_maxflow_mincut_freq;

public:

    /** default constructor */
    PCTSPconshdlrSubtour(
        SCIP* mip,
        bool _sec_disjoint_tour,
        int _sec_disjoint_tour_freq,
        bool _sec_maxflow_mincut,
        int _sec_maxflow_mincut_freq
    )
        : ObjConshdlr(mip, "subtour", "PCTSP subtour elimination constraints",
            1000000, -2000000, -2000000, 1, -1, 1, 0,
            FALSE, FALSE, TRUE, SCIP_PROPTIMING_BEFORELP, SCIP_PRESOLTIMING_FAST)
    {
        sec_disjoint_tour = _sec_disjoint_tour;
        sec_disjoint_tour_freq = _sec_disjoint_tour_freq;
        sec_maxflow_mincut = _sec_maxflow_mincut;
        sec_maxflow_mincut_freq = _sec_maxflow_mincut_freq;
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

/** Create a subtour elimination constraint
 *
 * This function mimics the SCIPcreateConsSubtour function in the TSP example
 * of the SCIP solver.
 */
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
);

SCIP_RETCODE PCTSPcreateBasicConsSubtour(
    SCIP* mip,
    SCIP_CONS** cons,
    std::string& name,
    PCTSPgraph& graph,
    PCTSPvertex& root_vertex
);

#endif