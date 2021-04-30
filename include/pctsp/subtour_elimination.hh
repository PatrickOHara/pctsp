#ifndef __PCTSP_SUBTOUR_ELIMINATION__
#define __PCTSP_SUBTOUR_ELIMINATION__

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
    PCTSPgraph& graph,
    std::vector<PCTSPvertex>& vertex_set,
    PCTSPedgeVariableMap& edge_variable_map,
    PCTSPvertex& root_vertex,
    PCTSPvertex& target_vertex);

void insertEdgeVertexVariables(VarVector& edge_variables,
    VarVector& vertex_variables,
    VarVector& all_variables,
    std::vector<double>& var_coefs
);

std::vector<PCTSPedge> getInducedEdges(PCTSPgraph& graph, std::vector<PCTSPvertex>& vertices);

#endif