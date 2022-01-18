/** A Boost graph defined with a prize vertex property and cost edge property */

#ifndef __PCTSP_GRAPH__
#define __PCTSP_GRAPH__
#include <boost/bimap.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
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

std::vector<SCIP_VAR*> getEdgeVariables(
    SCIP* scip,
    PCTSPgraph& graph,
    PCTSPedgeVariableMap& edge_variable_map,
    std::vector<PCTSPedge>& edges
);

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

std::vector<PCTSPedge> getSelfLoops(PCTSPgraph& graph, std::vector<PCTSPvertex>& vertices);

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

/**
 * @brief Count how many edges are in the filtered graph by iterating over every edge.
 * 
 * @tparam TGraph Graph type.
 * @tparam TFilter The edge filter
 * @param f_graph A filtered graph.
 * @return int Number of edges in the filtered graph.
 */
template <typename TGraph, typename TFilter>
int numEdgesInFilteredGraph(boost::filtered_graph<TGraph, TFilter>& f_graph) {
    int n_edges = 0;
    auto edges_it = boost::edges(f_graph);
    for (auto e : boost::make_iterator_range(boost::edges(f_graph))) {
        n_edges++;
    }
    return n_edges;
}

template <typename TPrizeMap, typename TPrizeType, typename TVertexIt>
bool isPrizeFeasible(
    TPrizeMap& prize_map,
    TPrizeType& quota,
    TVertexIt& first_vertex_it,
    TVertexIt& last_vertex_it
) {
    TPrizeType prize = 0;
    for (; first_vertex_it != last_vertex_it; first_vertex_it++) {
        prize += prize_map[*first_vertex_it];
    }
    return prize >= quota;
}

template <typename TPrizeMap, typename TPrizeType, typename TVertex>
bool isPrizeFeasible(
    TPrizeMap& prize_map,
    TPrizeType& quota,
    std::vector<TVertex>& vertex_vector
) {
    auto first = vertex_vector.begin();
    auto last = vertex_vector.end();
    return isPrizeFeasible(prize_map, quota, first, last);
}

/**
 * @brief Get the subpath of a cycle in a graph. Note first and last vertex are assumed to be the same.
 */
template <typename It>
std::vector<typename std::iterator_traits<It>::value_type> getSubpathOfCycle(
    It& first,
    It& last,
    int& subpath_start,
    int& subpath_end
) {
    typedef typename std::iterator_traits<It>::value_type TVertex;
    auto cycle_length = std::distance(first, last);
    auto subpath_length = subpath_end - subpath_start + 1;
    if (subpath_length <= 0) {
        subpath_length = cycle_length - subpath_start + subpath_end;
    }

    std::vector<TVertex> subpath (subpath_length);
    It current = first;
    std::advance(current, subpath_start);
 
    for (int i = 0; i < subpath_length; i++) {
        if (current == last) {
            current = first;
            current ++;
        }
        subpath[i] = *current;
        current ++;
    }
    return subpath;
}

std::vector<PCTSPvertex> getSubpathOfCycle(
    std::list<PCTSPvertex>& cycle,
    int& subpath_start,
    int& subpath_end
);


template <typename TGraph, typename TMarked, typename TParent>
void depthFirstSearch(
    TGraph& graph,
    typename TGraph::vertex_descriptor& source,
    TMarked& marked,
    TParent& parent,
    int depth_limit
) {
    marked[source] = true;
    if (depth_limit > 0) {
        for (auto neighbor: boost::make_iterator_range(boost::adjacent_vertices(source, graph))) {
            if (!marked[neighbor]) {
                parent[neighbor] = source;
                depthFirstSearch(graph, neighbor, marked, parent, depth_limit-1);
            }
        }
    }
}

template <typename TGraph, typename TMarked, typename TParent>
void breadthFirstSearch(
    TGraph& graph,
    typename TGraph::vertex_descriptor& source,
    TMarked& marked,
    TParent& parent,
    int depth_limit
) {
    marked[source] = true;
    parent[source] = source;
    std::list<typename TGraph::vertex_descriptor> queue = {source};
    int depth = 0;
    int this_level_counter = 1; // num vertices on this level of BFS tree
    int next_level_counter = 0; // num vertices on next level of BFS tree
    while (queue.size() > 0 && depth < depth_limit) {
        auto u = * queue.begin();
        queue.pop_front();
        this_level_counter --;
        for (auto neighbor: boost::make_iterator_range(boost::adjacent_vertices(u, graph))) {
            if (!marked[neighbor]) {
                marked[neighbor] = true;
                parent[neighbor] = u;
                queue.push_back(neighbor);
                next_level_counter ++;
            }
        }
        if (this_level_counter == 0) {
            this_level_counter = next_level_counter;
            depth ++;
            next_level_counter = 0;
        }
    }
}

template <typename TVertex, typename TParent>
std::list<TVertex> pathInTreeFromParents(
    TParent& parent_lookup,
    TVertex& source,
    TVertex& target
) {
    std::list<TVertex> path = {target};
    TVertex child = target;
    TVertex parent = parent_lookup[child];
    int i = 0;
    while (parent != child && i <= parent_lookup.size()) {
        path.push_front(parent);
        child = parent;
        parent = parent_lookup[parent];
        i++;
    }
    return path;
}


struct BoolVertexFilter {
    std::vector<bool> is_vertex_filtered;

    BoolVertexFilter(std::vector<bool>& filtered) : is_vertex_filtered(filtered) {}

    template <typename TVertex>
    bool operator() (const TVertex& u) const {
        return is_vertex_filtered[u];
    }
};

template <typename TGraph>
struct BoolEdgeFilter {
    std::vector<bool> is_vertex_filtered;
    TGraph * g;

    BoolEdgeFilter(TGraph& graph, std::vector<bool>& filtered) : is_vertex_filtered(filtered) {
        g = & graph;
    }

    template <typename TEdge>
    bool operator() (const TEdge& e) const {
        return is_vertex_filtered[boost::source(e, *g)] || is_vertex_filtered[boost::target(e, *g)];
    }
};

template <typename TGraph>
boost::filtered_graph<TGraph, BoolEdgeFilter<TGraph>, BoolVertexFilter>
filterMarkedVertices(TGraph& graph, std::vector<bool>& mark) {
    BoolVertexFilter v_filter (mark);
    BoolEdgeFilter<TGraph> e_filter (graph, mark);
    boost::filtered_graph<TGraph, BoolEdgeFilter<TGraph>, BoolVertexFilter> f_graph (graph, e_filter, v_filter);
    return f_graph;
}

#endif