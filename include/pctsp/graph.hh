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

typedef boost::adjacency_list<listS, vecS, undirectedS, PCTSPvertexProperties,
    PCTSPedgeProperties>
    PCTSPgraph;

typedef typename boost::graph_traits<PCTSPgraph>::edge_descriptor PCTSPedge;
// typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor PCTSPvertex;
typedef unsigned long PCTSPvertex;
typedef typename std::map<PCTSPvertex, int> PCTSPprizeMap;
typedef typename std::map<PCTSPedge, int> PCTSPcostMap;
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


template<typename Graph, typename EdgeIt>
void addEdgesToGraph(Graph& graph, EdgeIt& start, EdgeIt& end) {
    while (std::distance(start, end) > 0) {
        auto edge = *(start);
        boost::add_edge(edge.first, edge.second, graph);
        start++;
    }
}


template<typename Graph>
VertexPairVector getVertexPairVectorFromGraph(Graph& graph) {
    auto edges = getEdgeVectorOfGraph(graph);
    return getVertexPairVectorFromEdgeSubset(graph, edges);
}

std::vector <PCTSPedge> getEdgeVectorOfGraph(PCTSPgraph& graph);


VertexPairVector getVertexPairVectorFromEdgeSubset(
    PCTSPgraph& graph,
    std::vector < PCTSPedge> edge_subset_vector
);

/**
 * @brief Get the variables associated with each edge in the iterator
 */
template <typename Graph, typename EdgeVariableMap, typename EdgeIt>
std::vector<SCIP_VAR*> getEdgeVariables(
    SCIP* scip,
    Graph& graph,
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

template <typename Graph, typename EdgeIt>
std::vector<typename boost::graph_traits< Graph >::vertex_descriptor> getVerticesOfEdges(
    Graph& graph,
    EdgeIt& first,
    EdgeIt& last
) {
    typedef typename boost::graph_traits< Graph >::vertex_descriptor Vertex;
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

template <typename Graph, typename VertexIt>
std::vector<typename boost::graph_traits<Graph>::edge_descriptor> getSelfLoops(
    Graph& graph, VertexIt& first, VertexIt& last
) {
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
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