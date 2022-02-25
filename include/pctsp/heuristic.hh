#ifndef __PCTSP_HEURISTIC__
#define __PCTSP_HEURISTIC__

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <exception>
#include <iostream>
#include <limits>

#include <scip/scip.h>

#include "exception.hh"
#include "graph.hh"
#include "logger.hh"
#include "walk.hh"

using namespace boost;
using namespace std;

// Solver heuristics

void includeHeuristics(SCIP* scip);

// Extension and collapse heuristic

float unitaryGain(int prize_v, int cost_uw, int cost_uv, int cost_vw);

struct ExtensionVertex {
    int index;
    float value;
};


float unitaryLoss(
    int& external_path_prize,
    int& internal_path_prize,
    int& external_path_cost,
    int& internal_path_cost
);

/**
 * @brief Get the set intersection of the neighbors of vertex u and vertex v.
 */
template <typename TGraph>
std::vector<typename TGraph::vertex_descriptor> neighborIntersection(
    TGraph& graph,
    typename TGraph::vertex_descriptor& u,
    typename TGraph::vertex_descriptor& v
) {
    typedef typename TGraph::vertex_descriptor VertexDescriptor;
    typename graph_traits< TGraph >::adjacency_iterator ui, ui_end, vi, vi_end;
    boost::tie(ui, ui_end) = boost::adjacent_vertices(u, graph);
    boost::tie(vi, vi_end) = boost::adjacent_vertices(v, graph);
    std::vector<VertexDescriptor> u_neighbors (ui, ui_end);
    std::vector<VertexDescriptor> v_neighbors (vi, vi_end);
    std::vector<VertexDescriptor> intersection;
    std::sort(u_neighbors.begin(), u_neighbors.end());
    std::sort(v_neighbors.begin(), v_neighbors.end());
    std::set_intersection(u_neighbors.begin(), u_neighbors.end(), v_neighbors.begin(), v_neighbors.end(), std::back_inserter(intersection));
    return intersection;
}

template <typename TGraph, typename TCostMap, typename TPrizeMap>
ExtensionVertex chooseExtensionPathFromCandidates(
    TGraph& graph,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    std::vector<std::list<typename TGraph::vertex_descriptor>> external_path_candidates,
    std::vector<typename TGraph::vertex_descriptor> internal_path
) {
    ExtensionVertex best_candidate = {-1, 0.0};
    int current_index = 0;
    bool extension_found = false;
    for (auto& external_path : external_path_candidates) {
        // if the external path has prize greater than the internal path
        int internal_path_prize = totalPrize(prize_map, internal_path);
        int external_path_prize = total_prize(graph, external_path, prize_map);
        float prize_diff = external_path_prize - internal_path_prize;

        if (prize_diff > 0) {
            // then calculate the unitary loss of the external path between i and j
            int external_path_cost = totalCost(graph, external_path, cost_map);
            int internal_path_cost = totalCost(graph, internal_path, cost_map);
            auto loss_ij = unitaryLoss(
                external_path_prize,
                internal_path_prize,
                external_path_cost,
                internal_path_cost
            );
            if ((loss_ij < best_candidate.value) || !(extension_found)) {
                best_candidate.value = loss_ij;
                best_candidate.index = current_index;
                extension_found = true;
            }
        }
        current_index ++;
    }
    return best_candidate;
}

/**
 * @brief Replace the section of the tour from first to last index with the new path.
 * First and last index are inclusive!
 */
template<typename TVertex>
void swapPathsInTour(std::list<TVertex>& tour, std::list<TVertex>& new_path, int& first_index, int& last_index) {
    if (first_index < last_index) {
        // remove the path from first to last (inclusive)
        auto first_it = tour.begin();
        auto last_it = tour.begin();
        std::advance(first_it, first_index);
        std::advance(last_it, last_index);
        while (first_it != last_it) {
            first_it = tour.erase(first_it);
        }
        tour.erase(last_it);

        // insert the new path at the first index
        first_it = tour.begin();
        std::advance(first_it, first_index);
        tour.insert(first_it, new_path.begin(), new_path.end());
    }
    else {
        // remove the path from last to end of tour
        auto first_it = tour.begin();
        auto root_vertex = *first_it;        // remember the root vertex at the beginning and end of the tour... we need to replace this
        auto last_it = tour.end();
        std::advance(first_it, first_index);
        while (first_it != last_it) {
            first_it = tour.erase(first_it);
        }
        // remove the path from the beginning of the tour to the first index
        first_it = tour.begin();
        last_it = tour.begin();
        std::advance(last_it, last_index);
        while (first_it != last_it) {
            first_it = tour.erase(first_it);
        }
        tour.erase(last_it);

        auto root_it = std::find(new_path.begin(), new_path.end(),root_vertex);
        tour.insert(tour.begin(), root_it, new_path.end());
        root_it = std::find(new_path.begin(), new_path.end(),root_vertex);
        tour.insert(tour.end(), new_path.begin(), ++ root_it);
    }
}

int numFeasibleExtensions(std::vector<bool>& is_feasible_extension);

float averageUnitaryLoss(std::vector<float>& unitary_loss, std::vector<bool>& is_feasible_extension);

int indexOfSmallestLoss(std::vector<float>& unitary_loss, std::vector<bool>& is_feasible_extension);

template <typename TGraph, typename TCostMap, typename TPrizeMap>
void findExtensionPaths(
    TGraph& graph,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    typename TGraph::vertex_descriptor& root_vertex,
    int& step_size,
    int& path_depth_limit,
    std::vector<float>& unitary_loss,
    std::vector<bool>& is_feasible_extension,
    std::vector<std::list<typename TGraph::vertex_descriptor>>& extension_paths
) {
    typedef typename TGraph::vertex_descriptor VertexDescriptor;

    int k = tour.size() - 1;

    // create a set of vertices in tour
    std::unordered_set<VertexDescriptor> vertices_in_tour(std::begin(tour), std::end(tour));

    for (int i = 0; i < k; i++) {
        int j = (i + step_size) % k;
        auto it = tour.begin();
        std::advance(it, i);
        auto vi = *it;
        it = tour.begin();
        std::advance(it, j);
        auto vj = *it;
        // get the path in the tour from i to j
        auto start = tour.begin();
        auto end = tour.end();
        std::vector<VertexDescriptor> internal_path = getSubpathOfCycle(start, end, i, j);

        bool is_root_internal_vertex = isInternalVertexOfWalk(internal_path, root_vertex);

        // keep a vector of potential external paths
        std::vector<std::list<VertexDescriptor>> external_path_candidates;

        // check if the root is an internal vertex (not an endpoint) inside the internal path
        if ((path_depth_limit == 2) && is_root_internal_vertex) {
            if (boost::edge(vi, root_vertex, graph).second && boost::edge(root_vertex, vj, graph).second) {
                std::list<VertexDescriptor> path_iuj = {vi, root_vertex, vj};
                external_path_candidates.push_back(path_iuj);
            }
        }

        else if (path_depth_limit == 2) {
            // look at every vertex that is adjacent to both i and j
            auto neighbors = neighborIntersection(graph, vi, vj);
            for (const auto& u : neighbors) {
                if (vertices_in_tour.count(u) == 0) {
                    std::list<VertexDescriptor> path_iuj = {vi, u, vj};
                    external_path_candidates.push_back(path_iuj);
                }
            }
        }
        // need to check that root vertex is not an *internal* vertex between i and j
        else if (path_depth_limit > 2 && !(is_root_internal_vertex)) {
            // filter the graph by marking vertices that are already in the tour
            auto n_vertices = num_vertices(graph);
            std::vector<bool> marked (n_vertices);
            for (auto h: tour) marked[h] = true;
            // find a path using BFS from i to j using unmarked vertices
            std::vector<VertexDescriptor> parent (n_vertices);
            breadthFirstSearch(graph, vi, marked, parent, path_depth_limit - 1);

            for (auto neighbor : boost::make_iterator_range(boost::adjacent_vertices(vj, graph))) {
                if (vertices_in_tour.count(neighbor) == 0 && marked[neighbor]) {
                    auto path_ij = pathInTreeFromParents(parent, vi, neighbor);
                    path_ij.push_back(vj);

                    // path is only a candidate if it has prize larger than internal path
                    if (totalPrize(prize_map, path_ij) > totalPrize(prize_map, internal_path))
                        external_path_candidates.push_back(path_ij);
                }
            }
        }
        ExtensionVertex best_candidate = chooseExtensionPathFromCandidates(graph, cost_map, prize_map, external_path_candidates, internal_path);
        // store the best unitary loss of the path in the array
        if (best_candidate.index >= 0) {
            unitary_loss[i] = best_candidate.value;
            is_feasible_extension[i] = true;
            auto best_external_path = external_path_candidates[best_candidate.index];
            extension_paths[i] = std::list(best_external_path.begin(), best_external_path.end());
        }
        else {
            unitary_loss[i] = 0.0;
            is_feasible_extension[i] = false;
        }
    }
}

template <typename TGraph, typename TCostMap, typename TPrizeMap>
void extension(
    TGraph& graph,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    typename TGraph::vertex_descriptor& root_vertex,
    int& step_size,
    int& path_depth_limit
) {
    typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptor;

    bool exists_path_with_below_avg_loss = true;
    bool calculate_avg_loss = true;
    float avg_loss = 0.0;

    while (exists_path_with_below_avg_loss) {
        int k = tour.size() - 1;

        // define vectors to store unitary loss and paths
        std::vector<float> unitary_loss(k);
        std::vector<bool> is_feasible_extension(k);
        std::vector<std::list<VertexDescriptor>> extension_paths(k);

        // find all possible extension paths of length path_depth_limit
        findExtensionPaths(graph, tour, cost_map, prize_map, root_vertex, step_size, path_depth_limit, unitary_loss, is_feasible_extension, extension_paths);

        // get the number of possible extensions
        auto num_feasible_extensions = numFeasibleExtensions(is_feasible_extension);

        // calculate the average loss
        if (calculate_avg_loss) {
            avg_loss = averageUnitaryLoss(unitary_loss, is_feasible_extension);
            calculate_avg_loss = false;
        }

        // find the smallest loss in the array
        int index_of_smallest_loss = indexOfSmallestLoss(unitary_loss, is_feasible_extension);
        auto smallest_loss = unitary_loss[index_of_smallest_loss];

        // check if there exists a path with below average unitary loss
        exists_path_with_below_avg_loss = (smallest_loss < avg_loss) && (num_feasible_extensions > 0);

        // extend the tour with the path of smallest unitary loss
        if (exists_path_with_below_avg_loss) {
            auto external_path = extension_paths[index_of_smallest_loss];
            int last_index =  index_of_smallest_loss + step_size;
            swapPathsInTour(tour, external_path, index_of_smallest_loss, last_index);
        }
    }
}

template <typename TGraph, typename TCostMap, typename TPrizeMap>
void extension(
    TGraph& graph,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    typename TGraph::vertex_descriptor& root_vertex
) {
    int step_size = 1;
    int path_depth_limit = 2;
    return extension(graph, tour, cost_map, prize_map, root_vertex, step_size, path_depth_limit);
}

/**
 * @brief Extend the tour until it is prize-feasible.
 */
template <typename TGraph, typename TCostMap, typename TPrizeMap>
void extensionUntilPrizeFeasible(
    TGraph& graph,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    typename TGraph::vertex_descriptor& root_vertex,
    int& quota,
    int& step_size,
    int& path_depth_limit
) {
    typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptor;
    int prize = totalPrizeOfTour(graph, tour, prize_map);
    int num_feasible_extensions = 1;

    while (prize < quota && num_feasible_extensions > 0 && step_size < tour.size() - 1) {
        int k = tour.size() - 1;

        // define vectors to store unitary loss and paths
        std::vector<float> unitary_loss(k);
        std::vector<bool> is_feasible_extension(k);
        std::vector<std::list<VertexDescriptor>> extension_paths(k);

        // find all possible extension paths of length path_depth_limit
        findExtensionPaths(graph, tour, cost_map, prize_map, root_vertex, step_size, path_depth_limit, unitary_loss, is_feasible_extension, extension_paths);
        num_feasible_extensions = numFeasibleExtensions(is_feasible_extension);

        // extend the tour with the path of smallest unitary loss
        if (num_feasible_extensions > 0) {
            // find the smallest loss in the array
            int index_of_smallest_loss = indexOfSmallestLoss(unitary_loss, is_feasible_extension);
            auto smallest_loss = unitary_loss[index_of_smallest_loss];
            auto external_path = extension_paths[index_of_smallest_loss];
            int last_index = (index_of_smallest_loss + step_size) % k;
            swapPathsInTour(tour, external_path, index_of_smallest_loss, last_index);
        }
    }
}


template <typename TGraph, typename TCostMap, typename TPrizeMap>
ExtensionVertex unitaryGainOfVertex(
    TGraph& g,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TCostMap& cost_map,
    TPrizeMap& prize_map, typename TGraph::vertex_descriptor vertex
) {
    typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptor;
    typedef typename std::list<VertexDescriptor>::iterator tour_iterator_t;
    tour_iterator_t tour_iterator = tour.begin();
    VertexDescriptor prev_vertex_id = *tour_iterator;
    VertexDescriptor u = boost::vertex(prev_vertex_id, g);
    VertexDescriptor v = boost::vertex(vertex, g);

    // calculate the unitary gain of a vertex
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
            float gain = unitaryGain(prize_of_v, cost_uw, cost_uv, cost_vw);
            if (gain > max_gain) {
                max_gain = gain;
                index_of_extension = i;
            }
        }
        u = w;
        prev_vertex_id = *tour_iterator;
        i++;
    }
    ExtensionVertex gain_of_vertex = ExtensionVertex();
    gain_of_vertex.value = max_gain;
    gain_of_vertex.index = index_of_extension;
    return gain_of_vertex;
}

template <typename GainMap, typename VertexSet>
float calculateAverageGain(VertexSet& vertices_in_tour, GainMap& gain_map) {
    float total_gain = 0.0;
    int num_vertices_considered = 0;
    for (auto const& [key, val] : gain_map) {
        if (vertices_in_tour.count(key) == 0) {
            // only add up gain of vertices not in the tour
            total_gain += val.value;
            num_vertices_considered++;
        }
    }
    float avg_gain = total_gain / (float)(num_vertices_considered);
    return avg_gain;
}

template <typename TGraph, typename TCostMap, typename TPrizeMap, typename GainMap, typename VertexSet>
typename TGraph::vertex_descriptor findVertexWithBiggestGain(
    TGraph& graph,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    GainMap& gain_map,
    VertexSet& vertices_in_tour)
{
    // iterator over vertices in the graph
    typedef typename graph_traits<TGraph>::vertex_descriptor VertexDescriptor;
    typedef typename graph_traits<TGraph>::vertex_iterator vertex_iter;
    std::pair<vertex_iter, vertex_iter> vp;

    float biggest_gain = 0.0;
    VertexDescriptor biggest_gain_vertex;
    bool found_biggest_gain = false;
    // for each vertex not in the tour, calculate the unitary gain
    for (vp = vertices(graph); vp.first != vp.second; ++vp.first) {
        // only look at vertices not in the tour
        VertexDescriptor vertex = *vp.first;
        if (vertices_in_tour.count(vertex) == 0) {
            ExtensionVertex gain = unitaryGainOfVertex(
                graph, tour, cost_map, prize_map, vertex);
            gain_map[vertex] = gain;
            if (gain.value > biggest_gain) {
                biggest_gain_vertex = vertex;
                biggest_gain = gain.value;
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
void insertBiggestGainVertexIntoTour(
    Tour& tour,
    Vertex& biggest_gain_vertex,
    GainMap& gain_map
) {
    // initializing list iterator to beginning
    typedef typename Tour::iterator tour_iterator_t;
    tour_iterator_t tour_iterator = tour.begin();

    // iterator to point to position
    int insert_index = gain_map[biggest_gain_vertex].index + 1;
    std::advance(tour_iterator, insert_index);
    tour.insert(tour_iterator, biggest_gain_vertex);
}

// Extend a tour by adding vertices according to the unitary gain operation
template <typename TGraph, typename TCostMap, typename TPrizeMap>
void extend(
    TGraph& g,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TCostMap& cost_map,
    TPrizeMap& prize_map
) {
    typedef typename TGraph::vertex_descriptor VertexDescriptor;
    // we assume that the first and last vertex in the tour are the same

    // map vertices to the unitary gain
    typedef std::map<VertexDescriptor, ExtensionVertex> UnitaryGainMap;
    UnitaryGainMap gain_map;

    // create a set of vertices in tour
    std::unordered_set<VertexDescriptor> vertices_in_tour(std::begin(tour),
        std::end(tour));

    bool exists_vertices_with_above_avg_gain = true;
    bool calculate_avg_gain = true;
    float avg_gain = 0.0;

    while (exists_vertices_with_above_avg_gain) {
        try {
            VertexDescriptor biggest_gain_vertex = findVertexWithBiggestGain(
                g, tour, cost_map, prize_map, gain_map, vertices_in_tour);
            float biggest_gain = gain_map[biggest_gain_vertex].value;
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

template <typename TGraph, typename TCostMap, typename TPrizeMap>
void extendUntilPrizeFeasible(
    TGraph& g,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    int quota
) {
    // Run the extension algorithm until the total prize of the tour is greater
    // than the quota. We don't calculate the average gain, the only termination
    // criteria of the algorithm is that the tour has sufficient prize.
    // NOTE: there is no guarantee that a prize feasible tour is found!

    // define a mapping from vertices to the unitary gain data structure
    typedef typename TGraph::vertex_descriptor VertexDescriptor;
    typedef std::map<VertexDescriptor, ExtensionVertex> UnitaryGainMap;
    UnitaryGainMap gain_map;

    // create a set of vertices in tour
    std::unordered_set<VertexDescriptor> vertices_in_tour(std::begin(tour),
        std::end(tour));

    // keep track of the total prize of the tour
    int prize = totalPrizeOfTour(g, tour, prize_map);
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
            BOOST_LOG_TRIVIAL(info) << "Did not extend tour to be above quota";
        }
    }
}

template <typename ListType, typename ReverseIterator>
int indexOfReverseIterator(std::list<ListType>& my_list,
    ReverseIterator& reverse_it) {
    // get the current index of the reverse iterator
    return std::distance(std::begin(my_list), reverse_it.base()) - 1;
}

template <typename TVertex> struct SubPathOverTour {
    std::list<TVertex> path;
    int prize_of_path;
    TVertex feasibility_vertex;
    TVertex first_vertex;
    TVertex predecessor_vertex; // predecessor of the feasibility vertex in tour
    bool root_vertex_seen;
    bool feasible_path_found;
};

template <typename TVertex, typename TPrizeMap>
SubPathOverTour<TVertex>
getSubPathOverTour(std::list<TVertex>& tour, int index_of_first_vertex,
    TPrizeMap& prize_map, int quota, TVertex root_vertex) {
    // create an iterator over the tour to create a path
    typedef typename std::list<TVertex>::iterator tour_iterator_t;
    tour_iterator_t path_iterator = tour.begin();
    std::advance(path_iterator, index_of_first_vertex);

    // store variables in a struct
    SubPathOverTour<TVertex> sub_path = SubPathOverTour<TVertex>();
    sub_path.first_vertex = *path_iterator;
    sub_path.path = { sub_path.first_vertex };

    int length_of_tour = tour.size();

    // keep track of the prize of the path
    sub_path.prize_of_path = prize_map[sub_path.first_vertex];

    // keep track of important vertices
    TVertex prev_vertex = sub_path.first_vertex;
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
        TVertex current_vertex = *path_iterator;
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

/**
 * @brief Find an (external) path from the last vertex in the internal path
 * to the first vertex in the internal path.
 * 
 * The external path should not use any of the internal vertices of the internal path.
 * 
 * The number of vertices in the external path cannot exceed the depth_limit + 1.
 * A special case when the depth limit is two searches through neighboring vertices.
 */
template <typename TGraph, typename TCostMap, typename TPrizeMap>
std::list<std::list<typename TGraph::vertex_descriptor>> findCollapsePaths (
    TGraph& graph,
    std::list<typename TGraph::vertex_descriptor>& internal_path,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    int quota,
    bool collapse_shortest_paths
) {
    typedef typename TGraph::vertex_descriptor VD;
    VD source = internal_path.back();
    VD target = internal_path.front();
    auto prize_of_internal_path = totalPrize(prize_map, internal_path);
    std::list<std::list<VD>> collapse_paths;
    // keep track of which vertices are in the internal path
    int n = boost::num_vertices(graph);
    std::vector<bool> in_internal_path (n);
    for (auto u : internal_path) in_internal_path[u] = true;


    // for each neighbour of the source vertex
    for (VD collapse_vertex: boost::make_iterator_range(boost::adjacent_vertices(source, graph))) {
        // check if there exists an edge from v to root
        auto edge = boost::edge(collapse_vertex, target, graph);
        bool edge_exists = edge.second;
        auto prize_of_new_tour = prize_of_internal_path + prize_map[collapse_vertex];
        if (edge_exists && ! in_internal_path[collapse_vertex] && prize_of_new_tour >= quota) {
            std::list<VD> collapse = {source, collapse_vertex, target};
            collapse_paths.push_back(collapse);
        }
    }

    if (collapse_shortest_paths) {
        // mark unusable vertices
        typedef typename boost::filtered_graph<TGraph, BoolEdgeFilter<TGraph>, BoolVertexFilter> fgraph_t;
        std::vector<bool> mark (in_internal_path);
        mark[source] = false;
        mark[target] = false;

        // find the shortest path from the source to the target using unmarked vertices
        std::vector<int> distance (boost::num_vertices(graph));
        std::vector<VD> predecessor (boost::num_vertices(graph));
        std::vector<boost::default_color_type> color_map (boost::num_vertices(graph));
        auto v_index = boost::get(vertex_index, graph);


        try {
            dijkstraShortestPathBlacklist(graph, source, target, predecessor, distance, cost_map, color_map, mark);
        }
        catch (TargetVertexFound) {
            // get the shortest path from the source to the target
            auto color_start = color_map.begin();
            auto color_last = color_map.end();
            auto path_st = pathInTreeFromParents(predecessor, source, target);
            int prize_of_new_tour = prize_of_internal_path + totalPrize(prize_map, path_st) - prize_map[source] - prize_map[target];
            if (prize_of_new_tour >= quota) {
                collapse_paths.push_back(path_st);
            }
        }
    }
    return collapse_paths;
}

template <typename TGraph, typename TCostMap, typename TPrizeMap>
std::list<typename TGraph::vertex_descriptor> collapse(
    TGraph& graph,
    std::list<typename TGraph::vertex_descriptor>& tour,
    TCostMap& cost_map,
    TPrizeMap& prize_map,
    int quota,
    typename TGraph::vertex_descriptor root_vertex,
    bool collapse_shortest_paths = false
) {
    typedef typename TGraph::vertex_descriptor VertexDescriptor;
    typedef typename std::list<VertexDescriptor>::reverse_iterator reverse_tour_iterator_t;

    // store the least-cost prize-feasible tour
    int cost_of_best_tour = totalCost(graph, tour, cost_map);
    std::list<VertexDescriptor> best_tour(tour);

    // loop over the tour in reverse
    for (reverse_tour_iterator_t first_vertex_iterator = tour.rbegin();
        first_vertex_iterator != tour.rend(); ++first_vertex_iterator) {
        // create a path from the current position of the iterator
        int index_of_first_vertex =
            indexOfReverseIterator(tour, first_vertex_iterator);
        auto sub_path_over_tour = getSubPathOverTour(
            tour, index_of_first_vertex, prize_map, quota, root_vertex);


        if (sub_path_over_tour.root_vertex_seen && sub_path_over_tour.feasible_path_found) {
            // find potential collapses of the tour: all these paths will create prize feasible tours
            auto collapse_paths = findCollapsePaths(graph, sub_path_over_tour.path, cost_map, prize_map, quota, collapse_shortest_paths);

            auto internal_cost = totalCost(graph, sub_path_over_tour.path, cost_map);
            int best_index = -1;
            int i = 0;
            for (auto external_path : collapse_paths) {
                auto external_cost = totalCost(graph, external_path, cost_map);
                if (external_cost + internal_cost < cost_of_best_tour) {
                    best_index = i;
                    // update best cost
                    cost_of_best_tour = external_cost + internal_cost;
                }
                i++;
            }
            if (best_index >=0) {
                // copy internal path to best tour
                best_tour.assign(sub_path_over_tour.path.begin(), sub_path_over_tour.path.end());
                // append collapse path
                auto paths_it = collapse_paths.begin();
                std::advance(paths_it, best_index);
                auto external_path = * paths_it;
                for (auto external_it = ++ external_path.begin(); external_it != external_path.end(); external_it ++) {
                    best_tour.push_back(*external_it);
                }
                BOOST_LOG_TRIVIAL(debug) << "Collapse found a new best tour with cost " << cost_of_best_tour;

            }
        }
    }
    return ReorderTourFromRoot(best_tour, root_vertex);
}

#endif