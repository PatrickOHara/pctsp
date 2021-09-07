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

struct PCTSPvertexProperties {
    int prize;
};

struct PCTSPedgeProperties {
    int cost;
};

typedef int CostNumberType;
typedef int PrizeNumberType;

typedef boost::adjacency_list<listS, vecS, undirectedS,
    boost::property<vertex_distance_t, PrizeNumberType>,
    boost::property<edge_weight_t, CostNumberType>>
    PCTSPgraph;

typedef typename boost::graph_traits<PCTSPgraph>::edge_descriptor PCTSPedge;
typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor PCTSPvertex;
// typedef unsigned long PCTSPvertex;
// typedef typename std::map<PCTSPvertex, int> VertexPrizeMap;
// typedef typename boost::graph_traits<PCTSPgraph>::vertex_distance_t VertexPrize;
typedef typename boost::property_map<PCTSPgraph, vertex_distance_t>::type VertexPrizeMap;
// typedef typename std::map<PCTSPedge, int> EdgeCostMap;
typedef typename boost::property_map<PCTSPgraph, edge_weight_t>::type  EdgeCostMap;
typedef typename std::map<PCTSPedge, SCIP_VAR*> PCTSPedgeVariableMap;

typedef boost::bimap<PCTSPvertex, int> BoostPyBimap;
typedef boost::bimap<PCTSPvertex, PCTSPvertex> PCTSPbimap;

typedef std::pair<PCTSPvertex, PCTSPvertex> VertexPair;
typedef std::vector<VertexPair> VertexPairVector;
typedef std::vector<PCTSPvertex> PCTSPvertexVector;
typedef long CapacityType;
typedef std::vector<CapacityType> CapacityVector;
typedef std::map<VertexPair, CapacityType> StdCapacityMap;
typedef boost::property<boost::edge_weight_t, CapacityType> BoostCapacityMap;
typedef boost::adjacency_list <
    boost::vecS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property,
    BoostCapacityMap
>UndirectedCapacityGraph;

typedef adjacency_list_traits< vecS, vecS, directedS > Traits;
typedef boost::property< edge_reverse_t, Traits::edge_descriptor > ReverseEdges;
typedef boost::property< edge_residual_capacity_t, CapacityType, ReverseEdges> ResidualCapacityMap;
typedef boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::directedS,
    boost::no_property,
    property< edge_capacity_t, CapacityType, ResidualCapacityMap>> DirectedCapacityGraph;


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

#endif