#ifndef __PCTSP_WALK__
#define __PCTSP_WALK__

#include <algorithm>
#include <string>
#include <boost/graph/graph_traits.hpp>

#include "exception.hh"

using namespace boost;
using namespace std;

template <typename Vertex>
std::list<Vertex> ReorderTourFromRoot(std::list<Vertex>& tour,
    Vertex root_vertex) {
    // return a new tour with the root vertex at the start and end

    // NOTE the tour is assumed to start and end with the same vertex

    // define types
    typedef typename std::list<Vertex>::iterator tour_iterator_t;
    // get index of the root vertex in the list
    tour_iterator_t root_finder =
        std::find(tour.begin(), tour.end(), root_vertex);

    std::list<Vertex> new_tour;

    // root is the first vertex in the tour
    if (root_finder == tour.begin()) {
        new_tour = std::list<Vertex>(tour.begin(), tour.end());
    }
    else {
        new_tour = std::list<Vertex>(root_finder, tour.end());
        tour_iterator_t tour_it = tour.begin();
        ++tour_it; // skip first vertex - we have already added it
        for (; tour_it != root_finder; ++tour_it) {
            new_tour.push_back(*tour_it);
        }
        new_tour.push_back(root_vertex);
    }
    return new_tour;
}

template <typename TGraph, typename VertexIt>
std::vector<typename boost::graph_traits<TGraph>::edge_descriptor> getEdgesInWalk(
    TGraph& graph,
    VertexIt& first,
    VertexIt& last
) {
    typedef typename boost::graph_traits<TGraph>::edge_descriptor Edge;
    auto n_vertices = std::distance(first, last);
    if (n_vertices < 2) {
        return std::vector<Edge>();
    }
    std::vector<Edge> edges(n_vertices - 1);

    for (int i = 0; i < n_vertices - 1; i++) {
        auto prev_vertex = *first;
        first++;
        auto current_vertex = *first;
        auto edge = boost::edge(prev_vertex, current_vertex, graph);
        if (!edge.second) {
            throw EdgeNotFoundException(std::to_string(prev_vertex), std::to_string(current_vertex));
        }
        edges[i] = edge.first;
    }
    return edges;
}

template <typename TEdgeIt, typename TCostMap>
int totalCost(TEdgeIt& first, TEdgeIt& last, TCostMap& cost_map) {
    int cost = 0;
    for (; first != last; first++) {
        auto edge = *first;
        cost += cost_map[edge];
    }
    return cost;
}

template <typename TEdge, typename TCostMap>
int totalCost(std::vector<TEdge>& edges, TCostMap& cost_map) {
    auto first = edges.begin();
    auto last = edges.end();
    return totalCost(first, last, cost_map);
}

template <typename TEdge, typename TCostMap>
int totalCost(std::list<TEdge>& edges, TCostMap& cost_map) {
    auto first = edges.begin();
    auto last = edges.end();
    return totalCost(first, last, cost_map);
}

template <typename TGraph, typename TVertexIt, typename TCostMap>
int totalCost(TGraph& graph, TVertexIt& first_vertex_it, TVertexIt& last_vertex_it, TCostMap& cost_map) {
    auto edges = getEdgesInWalk(graph, first_vertex_it, last_vertex_it);
    auto first_edge_it = edges.begin();
    auto last_edge_it = edges.end();
    return totalCost(first_edge_it, last_edge_it, cost_map);
}

template <typename TGraph, typename TCostMap>
int totalCost(TGraph& graph, std::list<typename TGraph::vertex_descriptor>& tour, TCostMap& cost_map) {
    auto first = tour.begin();
    auto last = tour.end();
    return totalCost(graph, first, last, cost_map);
}

template <typename TGraph, typename TCostMap>
int totalCost(TGraph& graph, std::vector<typename TGraph::vertex_descriptor>& path, TCostMap& cost_map) {
    auto first = path.begin();
    auto last = path.end();
    return totalCost(graph, first, last, cost_map);
}

template<typename TPrizeMap, typename TVertexIt>
int totalPrize(TPrizeMap& prize_map, TVertexIt& first_vertex_it, TVertexIt& last_vertex_it) {
    int prize = 0;
    for (; first_vertex_it != last_vertex_it; first_vertex_it ++) {
        prize += prize_map[*first_vertex_it];
    }
    return prize;
}

template<typename TPrizeMap, typename TVertex>
int totalPrize(TPrizeMap& prize_map, std::list<TVertex>& path) {
    auto first = path.begin();
    auto last = path.end();
    return totalPrize(prize_map, first, last);
}

template<typename TPrizeMap, typename TVertex>
int totalPrize(TPrizeMap& prize_map, std::vector<TVertex>& path) {
    auto first = path.begin();
    auto last = path.end();
    return totalPrize(prize_map, first, last);
}

template <typename TGraph, typename TPrizeMap, typename Vertex>
int total_prize(TGraph& graph, std::list<Vertex>& tour, TPrizeMap& prize_map) {
    // calculate the total prize of a tour
    typedef typename std::list<Vertex>::iterator tour_iterator_t;
    int prize = 0;
    for (tour_iterator_t tour_iterator = tour.begin();
        tour_iterator != tour.end(); ++tour_iterator) {
        auto vertex = boost::vertex(*tour_iterator, graph);
        int prize_of_vertex = prize_map[vertex];
        prize += prize_of_vertex;
    }
    return prize;
}

template <typename TGraph, typename TPrizeMap>
int totalPrizeOfTour(
    TGraph& graph,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TPrizeMap& prize_map
) {
    // total prize of all vertices apart from the last vertex (which is
    // repeated)
    std::list<typename TGraph::vertex_descriptor> tour_copy(tour.begin(), --tour.end());
    return total_prize(graph, tour_copy, prize_map);
}

template <typename VertexIt>
std::string walkToString(VertexIt& first_it, VertexIt& last_it) {
    std::string walk_str = "";
    while (first_it != last_it) {
        walk_str += std::to_string(*first_it) + ", ";
        first_it ++;
    }
    return walk_str;
}

template <typename TVertex>
std::string walkToString(std::list<TVertex>& walk) {
    auto first = walk.begin();
    auto last = walk.end();
    return walkToString(first, last);
}

template <typename TVertex>
bool isInternalVertexOfWalk(std::vector<TVertex>& walk, TVertex& internal_vertex) {
    // returns true if the internal vertex is in the walk
    // AND if the internal vertex is not the first or last vertex in the walk
    auto start = walk.begin();
    auto end = walk.end();
    start++;
    end--;
    return std::find(start, end, internal_vertex) != end;
}

#endif