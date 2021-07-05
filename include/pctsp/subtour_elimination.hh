#ifndef __PCTSP_SUBTOUR_ELIMINATION__
#define __PCTSP_SUBTOUR_ELIMINATION__

#include "solution.hh"
#include "graph.hh"
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
    SCIP_RESULT* result         /**< pointer to store the result of the separation call */
);


// SCIP_RETCODE PCTSPseparateDisjointTour(
//     SCIP* scip,
//     SCIP_CONSHDLR* conshdlr,
//     PCTSPgraph& input_graph,
//     PCTSPgraph& support_graph,
//     PCTSPedgeVariableMap& edge_variable_map,
//     PCTSPvertex& root_vertex,
//     SCIP_SOL* sol,
//     SCIP_RESULT* result,
//     std::vector<int>& component,
//     int& n_components,
//     int& root_component,
//     int freq
// );

// SCIP_RETCODE PCTSPseparateMaxflowMincut(
//     SCIP* scip,
//     SCIP_CONSHDLR* conshdlr,
//     PCTSPgraph& input_graph,
//     PCTSPedgeVariableMap& edge_variable_map,
//     PCTSPvertex& root_vertex,
//     SCIP_SOL* sol,
//     SCIP_RESULT* result,
//     int freq
// );

void insertEdgeVertexVariables(VarVector& edge_variables,
    VarVector& vertex_variables,
    VarVector& all_variables,
    std::vector<double>& var_coefs
);

std::vector<PCTSPedge> getInducedEdges(PCTSPgraph& graph, std::vector<PCTSPvertex>& vertices);

/** SCIP user problem data for PCTSP */
class ProbDataPCTSP : public scip::ObjProbData
{
    int* quota_;
    PCTSPgraph* graph_;
    PCTSPvertex* root_vertex_;
    PCTSPedgeVariableMap* edge_variable_map_;
public:
    /** default constructor */
    ProbDataPCTSP(
        PCTSPgraph* graph,
        PCTSPvertex* root_vertex,
        PCTSPedgeVariableMap* edge_variable_map,
        int* quota
    )
    {
        graph_ = graph;
        root_vertex_ = root_vertex;
        edge_variable_map_ = edge_variable_map;
        quota_ = quota;
    };

    /** Get the input graph */
    PCTSPgraph* getInputGraph();

    /** Get the quota */
    int* getQuota();

    /** Get the root vertex */
    PCTSPvertex* getRootVertex();

    /** Get the mapping from edges to variables */
    PCTSPedgeVariableMap* getEdgeVariableMap();
};

class PCTSPconshdlrSubtour : public scip::ObjConshdlr
{
public:
    /** default constructor */
    PCTSPconshdlrSubtour(
        SCIP* mip
    )
        : ObjConshdlr(mip, "subtour", "PCTSP subtour elimination constraints",
            1000000, -2000000, -2000000, 1, -1, 1, 0,
            FALSE, FALSE, TRUE, SCIP_PROPTIMING_BEFORELP, SCIP_PRESOLTIMING_FAST)
    {
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