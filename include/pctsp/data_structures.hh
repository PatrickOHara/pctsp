#ifndef __PCTSP_DATA_STRUCTURES__
#define __PCTSP_DATA_STRUCTURES__

#include "graph.hh"
#include "stats.hh"

/** SCIP user problem data for PCTSP */
class ProbDataPCTSP : public scip::ObjProbData
{
    typedef typename boost::graph_traits<PCTSPgraph>::edge_descriptor Edge;
    typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor Vertex;
    typedef std::map<Edge, SCIP_VAR*> EdgeVarLookup;

    int* quota_;
    PCTSPgraph* graph_;
    Vertex* root_vertex_;
    EdgeVarLookup* edge_variable_map_;
    std::vector<NodeStats>* node_stats_;
public:
    /** default constructor */
    ProbDataPCTSP(
        PCTSPgraph* graph,
        Vertex* root_vertex,
        EdgeVarLookup* edge_variable_map,
        int* quota,
        std::vector<NodeStats>* node_stats
    )
    {
        graph_ = graph;
        root_vertex_ = root_vertex;
        edge_variable_map_ = edge_variable_map;
        quota_ = quota;
        node_stats_ = node_stats;
    };

    /** Get the input graph */
    PCTSPgraph* getInputGraph();

    /** Get the quota */
    int* getQuota();

    /** Get the root vertex */
    Vertex* getRootVertex();

    /** Get the mapping from edges to variables */
    EdgeVarLookup* getEdgeVariableMap();

    /** Get the node statistics */
    std::vector<NodeStats>* getNodeStats();
};

#endif