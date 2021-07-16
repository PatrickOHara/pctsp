#ifndef __PCTSP_WALK__
#define __PCTSP_WALK__

#include <algorithm>
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

template <typename Graph, typename VertexIt>
std::vector<typename boost::graph_traits<Graph>::edge_descriptor> getEdgesInWalk(
    Graph& graph,
    VertexIt& first,
    VertexIt& last
) {
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
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

template <typename Graph, typename CostMap, typename Vertex>
int total_cost(Graph& graph, std::list<Vertex>& tour, CostMap cost_map) {
    // if there are no edges in the tour, return zero
    if (tour.size() <= 1) {
        return 0;
    }
    // calculate the total cost of edges in a tour
    typedef typename std::list<Vertex>::iterator tour_iterator_t;
    tour_iterator_t tour_iterator = tour.begin();

    // keep track of the previous vertex
    Vertex prev_vertex = *tour_iterator;
    auto prev_vertex_descriptor = boost::vertex(prev_vertex, graph);
    // move to the next vertex in the tour
    ++tour_iterator;

    // keep track of the total cost
    int cost = 0;
    for (; tour_iterator != tour.end(); ++tour_iterator) {
        Vertex current_vertex = *tour_iterator;
        auto current_vertex_descriptor = boost::vertex(current_vertex, graph);
        auto edge = boost::edge(prev_vertex_descriptor,
            current_vertex_descriptor, graph);
        bool edge_exists = edge.second;
        if (edge_exists) {
            cost += cost_map[edge.first];
        }
        else {
            throw EdgeNotFoundException(std::to_string(prev_vertex),
                std::to_string(current_vertex));
        }
        prev_vertex = current_vertex;
        prev_vertex_descriptor = boost::vertex(prev_vertex, graph);
    }
    return cost;
}

template <typename Graph, typename PrizeMap, typename Vertex>
int total_prize(Graph& graph, std::list<Vertex>& tour, PrizeMap& prize_map) {
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

template <typename Graph, typename PrizeMap, typename Vertex>
int total_prize_of_tour(Graph& graph, std::list<Vertex>& tour,
    PrizeMap& prize_map) {
    // total prize of all vertices apart from the last vertex (which is
    // repeated)
    std::list<Vertex> tour_copy(tour.begin(), --tour.end());
    return total_prize(graph, tour_copy, prize_map);
}

#endif