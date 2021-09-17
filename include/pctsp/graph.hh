/** A Boost graph defined with a prize vertex property and cost edge property */

#ifndef __PCTSP_GRAPH__
#define __PCTSP_GRAPH__
#include <boost/bimap.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <objscip/objscip.h>

#include "pctsp/exception.hh"

using namespace boost;

// attribute types for cost and prize
typedef int CostNumberType;
typedef int PrizeNumberType;

// graph definition
typedef boost::adjacency_list<
    listS,
    vecS,
    undirectedS,        // graphs are undirected
    boost::property<vertex_distance_t, PrizeNumberType>,    // prize on vertices
    boost::property<edge_weight_t, CostNumberType>          // cost on edges
> PCTSPgraph;

// abbreviations for vertex and edge names
typedef typename boost::graph_traits<PCTSPgraph>::edge_descriptor PCTSPedge;
typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor PCTSPvertex;

// property maps for prize and cost
typedef typename boost::property_map<PCTSPgraph, vertex_distance_t>::type VertexPrizeMap;
typedef typename boost::property_map<PCTSPgraph, edge_weight_t>::type  EdgeCostMap;

// mapping from edges to SCIP variables
typedef typename std::map<PCTSPedge, SCIP_VAR*> PCTSPedgeVariableMap;

// a pair of vertices
typedef std::pair<PCTSPvertex, PCTSPvertex> VertexPair;

// a vector of vertex pairs
typedef std::vector<VertexPair> VertexPairVector;

// definitions for capacity graphs
typedef long CapacityType;
typedef std::vector<CapacityType> CapacityVector;

// useful generic functions for graphs

template<typename TGraph, typename EdgeIt>
void addEdgesToGraph(TGraph& graph, EdgeIt& start, EdgeIt& end) {
    while (std::distance(start, end) > 0) {
        auto edge = *(start);
        boost::add_edge(edge.first, edge.second, graph);
        start++;
    }
}


template<typename TGraph>
VertexPairVector getVertexPairVectorFromGraph(TGraph& graph) {
    auto edges = getEdgeVectorOfGraph(graph);
    return getVertexPairVectorFromEdgeSubset(graph, edges);
}

std::vector <PCTSPedge> getEdgeVectorOfGraph(PCTSPgraph& graph);


VertexPairVector getVertexPairVectorFromEdgeSubset(
    PCTSPgraph& graph,
    std::vector < PCTSPedge> edge_subset_vector
);

template< typename TGraph, typename PairIt>
std::vector<typename boost::graph_traits<TGraph>::edge_descriptor> edgesFromVertexPairs(
    TGraph& graph,
    PairIt& first,
    PairIt& last
) {
    typedef typename boost::graph_traits<TGraph>::edge_descriptor Edge;
    auto n_edges = std::distance(first, last);
    std::vector<Edge> edge_vector(n_edges);
    for (int i = 0; i < n_edges; i++) {
        auto pair = *first;
        auto u = pair.first;
        auto v = pair.second;
        auto edge = boost::edge(u, v, graph);
        if (!edge.second) {
            throw EdgeNotFoundException(std::to_string(u), std::to_string(v));
        }
        edge_vector[i] = edge.first;
        first++;
    }
    return edge_vector;
}

/**
 * @brief Get the variables associated with each edge in the iterator
 */
template <typename TGraph, typename EdgeVariableMap, typename EdgeIt>
std::vector<SCIP_VAR*> getEdgeVariables(
    SCIP* scip,
    TGraph& graph,
    EdgeVariableMap& edge_variable_map,
    EdgeIt& first,
    EdgeIt& last
) {
    auto n_edges = std::distance(first, last);
    std::vector<SCIP_VAR*> variables(n_edges);
    for (int i = 0; i < n_edges; i++) {
        SCIP_VAR* var = edge_variable_map[*first];
        if (var == NULL) {
            throw VariableIsNullException();
        }
        variables[i] = var;
        first++;
    }
    return variables;
}

template <typename TGraph, typename EdgeIt>
std::vector<typename boost::graph_traits< TGraph >::vertex_descriptor> getVerticesOfEdges(
    TGraph& graph,
    EdgeIt& first,
    EdgeIt& last
) {
    typedef typename boost::graph_traits< TGraph >::vertex_descriptor Vertex;
    std::set<Vertex> vertex_set;
    while (first != last) {
        auto edge = *first;
        Vertex u = boost::source(edge, graph);
        Vertex v = boost::target(edge, graph);
        if (vertex_set.count(u) == 0) {
            vertex_set.emplace(u);
        }
        if (vertex_set.count(v) == 0) {
            vertex_set.emplace(v);
        }
        first++;
    }
    int i = 0;
    std::vector<Vertex> vertex_vector(vertex_set.size());
    for (auto const& vertex : vertex_set) {
        vertex_vector[i++] = vertex;
    }
    return vertex_vector;
}

template <typename TGraph, typename VertexIt>
std::vector<typename boost::graph_traits<TGraph>::edge_descriptor> getSelfLoops(
    TGraph& graph, VertexIt& first, VertexIt& last
) {
    typedef typename boost::graph_traits<TGraph>::edge_descriptor Edge;
    auto n_vertices = std::distance(first, last);
    std::vector<Edge> edges(n_vertices);
    for (int i = 0; i < n_vertices; i++) {
        auto vertex = *first;
        auto pair = boost::edge(vertex, vertex, graph);
        if (!pair.second) {
            throw NoSelfLoopFoundException(std::to_string(vertex));
        }
        edges[i] = pair.first;
        first++;
    }
    return edges;
}

template <typename TGraph, typename TVertexIt>
std::vector<typename TGraph::edge_descriptor> getEdgesInducedByVertices(
    TGraph& graph, TVertexIt& first_vertex_it, TVertexIt& last_vertex_it
) {
    typedef typename TGraph::edge_descriptor TEdge;
    auto n_vertices = std::distance(first_vertex_it, last_vertex_it);
    std::vector<TEdge> edges;
    for (; first_vertex_it != last_vertex_it; first_vertex_it++) {
        for (auto it = std::next(first_vertex_it); it != last_vertex_it; it++) {
            auto vertex_source = *first_vertex_it;
            auto vertex_target = *it;
            auto potential_edge = boost::edge(vertex_source, vertex_target, graph);
            if (potential_edge.second) {
                edges.push_back(potential_edge.first);
            }
        }
    }
    return edges;
}

std::vector<PCTSPedge> getEdgesInducedByVertices(PCTSPgraph& graph, std::vector<PCTSPvertex>& vertices);

#endif