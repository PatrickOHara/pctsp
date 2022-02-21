#ifndef __PCTSP_SUBTOUR_ELIMINATION__
#define __PCTSP_SUBTOUR_ELIMINATION__

#include "data_structures.hh"
#include "graph.hh"
#include "renaming.hh"
#include "sciputils.hh"
#include "solution.hh"

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <objscip/objscip.h>

template <typename TGraph, typename TParityMap>
std::vector<typename boost::graph_traits<TGraph>::edge_descriptor> getEdgesFromCut(TGraph& graph, TParityMap& parity_map) {
    typedef typename boost::graph_traits<TGraph>::edge_descriptor Edge;
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

bool isSolSimpleCycle(SCIP* scip, SCIP_SOL* sol, SCIP_RESULT* result);

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
    bool sec_maxflow_mincut
);

template <typename TEdgeWeightMap>
struct positive_edge_weight {
    positive_edge_weight() { }
    positive_edge_weight(TEdgeWeightMap weight) : m_weight(weight) { }
    template <typename TEdge>
    bool operator()(const TEdge& e) const {
        return 0 < get(m_weight, e);
    }
    TEdgeWeightMap m_weight;
};


template<typename TGraph, typename TWeight>
std::vector<typename TGraph::vertex_descriptor> getReachableVertices(
    TGraph& graph,
    typename boost::graph_traits<TGraph>::vertex_descriptor& source_vertex,
    TWeight& weight
) {
    typedef typename boost::graph_traits<TGraph>::vertex_descriptor Vertex;
    std::vector<Vertex> reachable_vertices;

    // filter the graph to get edges that have positive weight
    positive_edge_weight<TWeight> filter(weight);
    boost::filtered_graph<TGraph, positive_edge_weight<TWeight> > f_graph(graph, filter);

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
        if (colormap[vertex] == source_color) {
            reachable_vertices.push_back(vertex);
        }
    }
    return reachable_vertices;
}

template<typename TGraph, typename TWeight>
std::vector<typename boost::graph_traits<TGraph>::vertex_descriptor> getUnreachableVertices(
    TGraph& graph,
    typename boost::graph_traits<TGraph>::vertex_descriptor& source_vertex,
    TWeight& weight
) {
    typedef typename boost::graph_traits<TGraph>::vertex_descriptor Vertex;
    std::vector<Vertex> unreachable_vertices;

    // filter the graph to get edges that have positive weight
    positive_edge_weight<TWeight> filter(weight);
    boost::filtered_graph<TGraph, positive_edge_weight<TWeight> > f_graph(graph, filter);

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

template <typename TGraph, typename TEdgeVariableMap>
SCIP_RETCODE PCTSPseparateDisjointTour(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    TGraph& input_graph,
    TEdgeVariableMap& edge_variable_map,
    typename TGraph::vertex_descriptor& root_vertex,
    std::vector<std::vector<typename TGraph::vertex_descriptor>>& component_vectors,
    SCIP_SOL* sol,
    SCIP_RESULT* result,
    int& root_component_id,
    int& num_conss_added
) {
    auto n_components = component_vectors.size();
    if (n_components == 1) return SCIP_OKAY;
    for (int component_id = 0; component_id < n_components; component_id++) {
        auto support_vertex_set = component_vectors[component_id];
        if (component_id != root_component_id && support_vertex_set.size() >= 2) {
            std::vector<typename TGraph::vertex_descriptor> vertex_set(support_vertex_set.size());
            int i = 0;
            // get the vertex objects from the original graph
            for (auto const& solution_vertex : support_vertex_set) {
                vertex_set[i] = boost::vertex(solution_vertex, input_graph);
                i++;
            }
            for (auto target_vertex: vertex_set) {
                SCIP_CALL(addSubtourEliminationConstraint(
                    scip,
                    conshdlr,
                    input_graph,
                    vertex_set,
                    edge_variable_map,
                    root_vertex,
                    target_vertex,
                    sol,
                    result
                ));
                num_conss_added ++;
            }
        }
    }
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
    int& num_conss_added,
    int freq
);

void pushIntoRollingLpGapList(std::list<double>& rolling_gaps, double& gap, int& sec_max_tailing_off_iterations);

bool isNodeTailingOff(
    std::list<double>& rolling_gaps,
    double& sec_lp_gap_improvement_threshold,
    int& sec_max_tailing_off_iterations
);

const std::string SEC_CONSHDLR_NAME = "pctsp_sec_handler";
const std::string SEC_CONSHDLR_DESC = "Subtour elimination constraint handler for Prize-collecting TSP.";
const int SEC_CONSHDLR_SEPAPRIORITY = 1000000;  // used to be 1000000
const int SEC_CONSHDLR_ENFOPRIORITY = 1000000; // used to be -2000000
const int SEC_CONSHDLR_CHECKPRIORITY = -2000000; // used to be -2000000
const int SEC_CONSHDLR_SEPAFREQ = 1;    // 1 : > 1 reduces number of SECs added
const int SEC_CONSHDLR_PROPFREQ = -1;   // -1
const int SEC_CONSHDLR_EAGERFREQ = 1;   // used to be 1
const int SEC_CONSHDLR_MAXPREROUNDS = 0;
const bool SEC_CONSHDLR_DELAYSEPA = false; // false
const bool SEC_CONSHDLR_DELAYPROP = false;
const bool SEC_CONSHDLR_NEEDSCONS = false;

class PCTSPconshdlrSubtour : public scip::ObjConshdlr
{

private:

    std::vector<std::list<double>> node_rolling_lp_gap;
    bool sec_disjoint_tour;
    double sec_lp_gap_improvement_threshold;
    bool sec_maxflow_mincut;
    int sec_max_tailing_off_iterations;

public:

    /** default constructor */
    PCTSPconshdlrSubtour(
        SCIP* scip,
        bool _sec_disjoint_tour,
        double _sec_lp_gap_improvement_threshold,
        bool _sec_maxflow_mincut,
        int _sec_max_tailing_off_iterations,
        int _sec_sepafreq
    )
        : ObjConshdlr(scip, SEC_CONSHDLR_NAME.c_str(), SEC_CONSHDLR_DESC.c_str(),
            SEC_CONSHDLR_SEPAPRIORITY, SEC_CONSHDLR_ENFOPRIORITY, SEC_CONSHDLR_CHECKPRIORITY,
            _sec_sepafreq, SEC_CONSHDLR_PROPFREQ, SEC_CONSHDLR_EAGERFREQ, SEC_CONSHDLR_MAXPREROUNDS,
            SEC_CONSHDLR_DELAYSEPA, SEC_CONSHDLR_DELAYPROP, SEC_CONSHDLR_NEEDSCONS,
            SCIP_PROPTIMING_BEFORELP, SCIP_PRESOLTIMING_FAST)
    {
        sec_disjoint_tour = _sec_disjoint_tour;
        sec_lp_gap_improvement_threshold = _sec_lp_gap_improvement_threshold;
        sec_maxflow_mincut = _sec_maxflow_mincut;
        sec_max_tailing_off_iterations = _sec_max_tailing_off_iterations;
        node_rolling_lp_gap = {};
    }

    PCTSPconshdlrSubtour(SCIP* scip, bool _sec_disjoint_tour, bool _sec_maxflow_mincut)
        : PCTSPconshdlrSubtour(scip, _sec_disjoint_tour, 0.0, _sec_maxflow_mincut, -1, 1) {}

    

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
);

SCIP_RETCODE PCTSPcreateBasicConsSubtour(
    SCIP* scip,
    SCIP_CONS** cons,
    std::string& name,
    PCTSPgraph& graph,
    PCTSPvertex& root_vertex
);

#endif