#ifndef __PCTSP_HEURISTIC__
#define __PCTSP_HEURISTIC__

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <exception>
#include <iostream>
#include <limits>

#include "logger.hh"
#include "walk.hh"

using namespace boost;
using namespace std;

// Extension and collapse heuristic

float unitary_gain(int prize_v, int cost_uw, int cost_uv, int cost_vw);

struct UnitaryGainOfVertex {
    int index_of_extension;
    float gain_of_vertex;
};

template <typename Graph, typename CostMap, typename PrizeMap, typename T>
UnitaryGainOfVertex unitaryGainOfVertex(Graph& g, std::list<T>& tour,
    CostMap& cost_map,
    PrizeMap& prize_map, T vertex) {
    // typedef typename Graph::edge_descriptor Edge;
    typedef
        typename boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;
    typedef typename std::list<T>::iterator tour_iterator_t;
    tour_iterator_t tour_iterator = tour.begin();
    T prev_vertex_id = *tour_iterator;
    VertexDescriptor u = boost::vertex(prev_vertex_id, g);
    VertexDescriptor v = boost::vertex(vertex, g);

    // caclulate the unitary gain of a vertex
    float max_gain = 0.0;
    int prize_of_v = prize_map[v];
    int index_of_extension;
    // iterate over the endpoints of each edge in the tour to find max unitary
    // gain
    int i = 0;
    ++tour_iterator;
    for (; tour_iterator != tour.end(); ++tour_iterator) {
        VertexDescriptor w = boost::vertex(*tour_iterator, g);
        auto edge_uv = edge(u, v, g).first;
        bool uv_exists = edge(u, v, g).second;
        auto edge_vw = edge(v, w, g).first;
        bool vw_exists = edge(v, w, g).second;
        auto edge_uw = edge(u, w, g).first; // edge in tour, assume it exists
        bool uw_exists = edge(u, w, g).second;
        if (!uw_exists) {
            string error_message =
                "Edge between " + std::to_string(prev_vertex_id) + " and " +
                std::to_string(*tour_iterator) + " does not exist. \n";
            throw std::invalid_argument(error_message);
        }

        // check edges exist from (u, vertex) and (vertex, v)
        if ((vw_exists) && (uv_exists)) {
            int cost_uv = cost_map[edge_uv];
            int cost_vw = cost_map[edge_vw];
            int cost_uw = cost_map[edge_uw];
            float gain = unitary_gain(prize_of_v, cost_uw, cost_uv, cost_vw);
            if (gain > max_gain) {
                max_gain = gain;
                index_of_extension = i;
            }
        }
        u = w;
        prev_vertex_id = *tour_iterator;
        i++;
    }
    UnitaryGainOfVertex gain_of_vertex = UnitaryGainOfVertex();
    gain_of_vertex.gain_of_vertex = max_gain;
    gain_of_vertex.index_of_extension = index_of_extension;
    return gain_of_vertex;
}

template <typename GainMap, typename VertexSet>
float calculateAverageGain(VertexSet& vertices_in_tour, GainMap& gain_map) {
    float total_gain = 0.0;
    int num_vertices_considered = 0;
    for (auto const& [key, val] : gain_map) {
        if (vertices_in_tour.count(key) == 0) {
            // only add up gain of vertices not in the tour
            total_gain += val.gain_of_vertex;
            num_vertices_considered++;
        }
    }
    float avg_gain = total_gain / (float)(num_vertices_considered);
    return avg_gain;
}

template <typename Graph, typename Vertex, typename CostMap, typename PrizeMap,
    typename GainMap, typename VertexSet>
    Vertex findVertexWithBiggestGain(Graph& graph, std::list<Vertex>& tour,
        CostMap& cost_map, PrizeMap& prize_map,
        GainMap& gain_map,
        VertexSet& vertices_in_tour) {
    // iterator over vertices in the graph
    typedef typename graph_traits<Graph>::vertex_iterator vertex_iter;
    std::pair<vertex_iter, vertex_iter> vp;

    float biggest_gain = 0.0;
    Vertex biggest_gain_vertex;
    bool found_biggest_gain = false;
    // for each vertex not in the tour, calculate the unitary gain
    for (vp = vertices(graph); vp.first != vp.second; ++vp.first) {
        // only look at vertices not in the tour
        Vertex vertex = *vp.first;
        if (vertices_in_tour.count(vertex) == 0) {
            UnitaryGainOfVertex gain = unitaryGainOfVertex(
                graph, tour, cost_map, prize_map, vertex);
            gain_map[vertex] = gain;
            if (gain.gain_of_vertex > biggest_gain) {
                biggest_gain_vertex = vertex;
                biggest_gain = gain.gain_of_vertex;
                found_biggest_gain = true;
            }
        }
    }
    if (!found_biggest_gain) {
        throw NoGainVertexFoundException();
    }
    return biggest_gain_vertex;
}

template <typename GainMap, typename Tour, typename Vertex>
void insertBiggestGainVertexIntoTour(Tour& tour,
    Vertex& biggest_gain_vertex,
    GainMap& gain_map) {
    // initializing list iterator to beginning
    typedef typename Tour::iterator tour_iterator_t;
    tour_iterator_t tour_iterator = tour.begin();

    // iterator to point to position
    int insert_index = gain_map[biggest_gain_vertex].index_of_extension + 1;
    std::advance(tour_iterator, insert_index);
    tour.insert(tour_iterator, biggest_gain_vertex);
}

// Extend a tour by adding vertices according to the unitary gain operation
template <typename Graph, typename Vertex, typename CostMap, typename PrizeMap>
void extend(Graph& g, std::list<Vertex>& tour, CostMap& cost_map,
    PrizeMap& prize_map) {
    // we assume that the first and last vertex in the tour are the same

    // map vertices to the unitary gain
    typedef std::map<Vertex, UnitaryGainOfVertex> UnitaryGainMap;
    UnitaryGainMap gain_map;

    // create a set of vertices in tour
    std::unordered_set<Vertex> vertices_in_tour(std::begin(tour),
        std::end(tour));

    bool exists_vertices_with_above_avg_gain = true;
    bool calculate_avg_gain = true;
    float avg_gain = 0.0;

    while (exists_vertices_with_above_avg_gain) {
        try {
            Vertex biggest_gain_vertex = findVertexWithBiggestGain(
                g, tour, cost_map, prize_map, gain_map, vertices_in_tour);
            float biggest_gain = gain_map[biggest_gain_vertex].gain_of_vertex;
            // only calculate the average gain once
            if (calculate_avg_gain) {
                avg_gain = calculateAverageGain(vertices_in_tour, gain_map);
                calculate_avg_gain = false;
            }
            // add vertex with biggest gain to tour if it has above average gain
            if (biggest_gain > avg_gain) {
                insertBiggestGainVertexIntoTour(tour, biggest_gain_vertex,
                    gain_map);
                vertices_in_tour.insert(biggest_gain_vertex);
            }
            else {
                exists_vertices_with_above_avg_gain = false;
            }
        }
        catch (NoGainVertexFoundException e) {
            exists_vertices_with_above_avg_gain = false;
        }
    }
}

template <typename Graph, typename Vertex, typename CostMap, typename PrizeMap>
void extendUntilPrizeFeasible(Graph& g, std::list<Vertex>& tour,
    CostMap& cost_map, PrizeMap& prize_map,
    int quota) {
    // Run the extension algorithm until the total prize of the tour is greater
    // than the quota. We don't calculate the average gain, the only termination
    // criteria of the algorithm is that the tour has sufficient prize.
    // NOTE: there is no gaurantee that a prize feasible tour is found!

    // define a mapping from vertices to the unitary gain data structure
    typedef std::map<Vertex, UnitaryGainOfVertex> UnitaryGainMap;
    UnitaryGainMap gain_map;

    // create a set of vertices in tour
    std::unordered_set<Vertex> vertices_in_tour(std::begin(tour),
        std::end(tour));

    // keep track of the total prize of the tour
    int prize = total_prize_of_tour(g, tour, prize_map);
    int attempts = 0;
    bool insert_a_vertex = true;
    while (prize < quota && insert_a_vertex) {
        try {
            auto biggest_gain_vertex = findVertexWithBiggestGain(
                g, tour, cost_map, prize_map, gain_map, vertices_in_tour);
            if (insert_a_vertex) {
                insertBiggestGainVertexIntoTour(tour, biggest_gain_vertex,
                    gain_map);
                vertices_in_tour.insert(biggest_gain_vertex);
                attempts++;
                prize += prize_map[biggest_gain_vertex];
            }
            insert_a_vertex = attempts <= boost::num_vertices(g);
        }
        catch (NoGainVertexFoundException e) {
            insert_a_vertex = false;
            cerr << "Did not extend tour to be above quota";
        }
    }
}

template <typename ListType, typename ReverseIterator>
int indexOfReverseIterator(std::list<ListType>& my_list,
    ReverseIterator& reverse_it) {
    // get the current index of the reverse iterator
    return std::distance(std::begin(my_list), reverse_it.base()) - 1;
}

template <typename Vertex> struct SubPathOverTour {
    std::list<Vertex> path;
    int prize_of_path;
    Vertex feasibility_vertex;
    Vertex first_vertex;
    Vertex predecessor_vertex; // predecessor of the feasibility vertex in tour
    bool root_vertex_seen;
    bool feasible_path_found;
};

template <typename Vertex, typename PrizeMap>
SubPathOverTour<Vertex>
getSubPathOverTour(std::list<Vertex>& tour, int index_of_first_vertex,
    PrizeMap& prize_map, int quota, Vertex root_vertex) {
    // create an iterator over the tour to create a path
    typedef typename std::list<Vertex>::iterator tour_iterator_t;
    tour_iterator_t path_iterator = tour.begin();
    std::advance(path_iterator, index_of_first_vertex);

    // store variables in a struct
    SubPathOverTour<Vertex> sub_path = SubPathOverTour<Vertex>();
    sub_path.first_vertex = *path_iterator;
    sub_path.path = { sub_path.first_vertex };

    int length_of_tour = tour.size();

    // keep track of the prize of the path
    sub_path.prize_of_path = prize_map[sub_path.first_vertex];

    // keep track of important vertices
    Vertex prev_vertex = sub_path.first_vertex;
    sub_path.feasible_path_found = false;
    path_iterator = std::next(path_iterator);
    sub_path.root_vertex_seen = sub_path.first_vertex == root_vertex;

    BOOST_LOG_TRIVIAL(debug) << "Start vertex is " << sub_path.first_vertex;

    // build a path from the first vertex in the path
    // when the prize of the path is feasible, break the loop
    int length_of_path = 1;
    for (; (length_of_path < length_of_tour - 1) & (sub_path.prize_of_path < quota);
        length_of_path++) {
        BOOST_LOG_TRIVIAL(debug) << ". Started building path";
        if (path_iterator == tour.end()) {
            path_iterator = tour.begin(); // loop back to start of tour
            ++path_iterator; // first vertex is the same as last vertex
        }
        Vertex current_vertex = *path_iterator;
        int prize_of_current_vertex = prize_map[current_vertex];
        if (sub_path.prize_of_path + prize_of_current_vertex >= quota) {
            sub_path.feasibility_vertex = current_vertex;
            sub_path.predecessor_vertex = prev_vertex;
            sub_path.feasible_path_found = true;
            break;
        }
        else {
            sub_path.prize_of_path += prize_of_current_vertex;
            sub_path.path.push_back(current_vertex);
        }
        if (current_vertex == root_vertex) {
            sub_path.root_vertex_seen = true;
        }
        ++path_iterator;
        prev_vertex = current_vertex;
    }
    return sub_path;
}

template <typename Graph, typename Vertex, typename CostMap, typename PrizeMap>
std::list<Vertex> collapse(Graph& graph, std::list<Vertex>& tour,
    CostMap& cost_map, PrizeMap& prize_map, int quota,
    Vertex root_vertex) {

    typedef
        typename std::list<Vertex>::reverse_iterator reverse_tour_iterator_t;

    // store the least-cost prize-feasible tour
    int cost_of_best_tour = total_cost(graph, tour, cost_map);
    std::list<Vertex> best_tour(tour);

    // loop over the tour in reverse
    for (reverse_tour_iterator_t first_vertex_iterator = tour.rbegin();
        first_vertex_iterator != tour.rend(); ++first_vertex_iterator) {
        // create a path from the current position of the iterator
        int index_of_first_vertex =
            indexOfReverseIterator(tour, first_vertex_iterator);
        auto sub_path_over_tour = getSubPathOverTour(
            tour, index_of_first_vertex, prize_map, quota, root_vertex);

        // look for collapse vertices from the predecessor of the feasibility
        // vertex
        BOOST_LOG_TRIVIAL(debug) << ". Prize of path " << sub_path_over_tour.prize_of_path;
        BOOST_LOG_TRIVIAL(debug) << ". Feasibility vertex is "
            << sub_path_over_tour.feasibility_vertex;
        BOOST_LOG_TRIVIAL(debug) << ". Predecessor vertex is "
            << sub_path_over_tour.predecessor_vertex;
        BOOST_LOG_TRIVIAL(debug) << ". Root seen: " << sub_path_over_tour.root_vertex_seen;
        BOOST_LOG_TRIVIAL(debug) << ". Feasible path found: "
            << sub_path_over_tour.feasible_path_found;

        if (sub_path_over_tour.root_vertex_seen &&
            sub_path_over_tour.feasible_path_found) {

            auto predecessor_vertex_descriptor =
                boost::vertex(sub_path_over_tour.predecessor_vertex, graph);
            auto first_vertex_descriptor =
                boost::vertex(sub_path_over_tour.first_vertex, graph);
            int cost_of_path =
                total_cost(graph, sub_path_over_tour.path, cost_map);

            // keep set of vertices in path
            std::unordered_set<Vertex> vertices_in_path(
                std::begin(sub_path_over_tour.path),
                std::end(sub_path_over_tour.path));

            // for each neighbour v of the predecessor vertex
            for (Vertex collapse_vertex :
            make_iterator_range(boost::adjacent_vertices(
                sub_path_over_tour.predecessor_vertex, graph))) {
                auto collapse_vertex_descriptor =
                    boost::vertex(collapse_vertex, graph);

                // check if there exists an edge from v to root
                auto edge = boost::edge(collapse_vertex_descriptor,
                    first_vertex_descriptor, graph);
                bool edge_exists = edge.second;
                // ensure collapse vertex has not yet been seen in path
                if (edge_exists &
                    vertices_in_path.count(collapse_vertex) == 0) {

                    // calculate the prize of the new tour
                    int prize_of_new_tour = sub_path_over_tour.prize_of_path +
                        prize_map[collapse_vertex];
                    // calculate the cost of the new tour
                    auto edge_from_predecessor_to_collapse =
                        boost::edge(predecessor_vertex_descriptor,
                            collapse_vertex_descriptor, graph);
                    int cost_of_predecessor_to_collapse =
                        cost_map[edge_from_predecessor_to_collapse.first];
                    int cost_of_collapse_to_first = cost_map[edge.first];
                    int cost_of_new_tour = cost_of_path +
                        cost_of_predecessor_to_collapse +
                        cost_of_collapse_to_first;

                    BOOST_LOG_TRIVIAL(debug) << "Start at " << sub_path_over_tour.first_vertex;
                    BOOST_LOG_TRIVIAL(debug) << ". Predecessor at " << vertex_predecessor;
                    BOOST_LOG_TRIVIAL(debug) << ". Collapse at " << collapse_vertex;
                    BOOST_LOG_TRIVIAL(debug) << ". New cost is " << cost_of_new_tour;
                    BOOST_LOG_TRIVIAL(debug) << "(current best is " << cost_of_best_tour;
                    BOOST_LOG_TRIVIAL(debug) << "). New prize is " << prize_of_new_tour;

                    // check if the induced cycle is prize feasible
                    // check if the cost is better than the best known cost
                    if (prize_of_new_tour >= quota &
                        cost_of_new_tour < cost_of_best_tour) {
                        BOOST_LOG_TRIVIAL(debug) << ". NEW BEST TOUR!";
                        // copy path to best tour
                        best_tour.assign(sub_path_over_tour.path.begin(),
                            sub_path_over_tour.path.end());
                        // append collapse and first vertex to path
                        best_tour.push_back(collapse_vertex);
                        best_tour.push_back(sub_path_over_tour.first_vertex);
                        // update best cost
                        cost_of_best_tour = cost_of_new_tour;
                    }
                    BOOST_LOG_TRIVIAL(debug);
                }
            }
        }
    }
    return ReorderTourFromRoot(best_tour, root_vertex);
}
#endif