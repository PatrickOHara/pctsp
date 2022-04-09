"""Heuristics for the Prize-Collecting TSP"""

import logging
import random
import sys
from typing import Mapping
import networkx as nx
from tspwplib import (
    DisjointPaths,
    EdgeFunctionName,
    Vertex,
    VertexFunction,
    VertexFunctionName,
    VertexList,
    edge_list_from_walk,
    is_pctsp_yes_instance,
    total_cost,
    total_prize_of_tour,
)

# pylint: disable=import-error
from .libpypctsp import (
    collapse_bind,
    extension_bind,
    extension_until_prize_feasible_bind,
    path_extension_collapse_bind,
)

# pylint: enable=import-error


def collapse(
    graph: nx.Graph,
    tour: VertexList,
    quota: int,
    root_vertex: Vertex,
    collapse_shortest_paths: bool = False,
    logging_level: int = logging.INFO,
) -> VertexList:
    """Collapse the tour by finding shortcuts

    Args:
        graph: Undirected input graph with edge costs and vertex prizes
        tour: Tour that has the first and last vertex the same
        quota: The minimum prize the tour must collect
        root_vertex: The tour must start and end at the root vertex
        collapse_shortest_paths: Find collapse paths as well as collapse vertices
        logging_level: Verbosity of logging.

    Returns:
        A collapsed, prize-feasible tour that has at most the same cost as the input tour
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edge_list = list(graph.edges())
    collapsed_tour: VertexList = collapse_bind(
        edge_list,
        tour,
        cost_dict,
        prize_dict,
        quota,
        root_vertex,
        collapse_shortest_paths,
        logging_level,
    )
    return collapsed_tour


def extension(
    graph: nx.Graph,
    tour: VertexList,
    root_vertex: int,
    step_size: int = 1,
    path_depth_limit: int = 2,
    logging_level: int = logging.INFO,
) -> VertexList:
    """Increase the prize of the tour by selecting vertices according to their unitary loss.

    Args:
        graph: Undirected input graph
        tour: Tour that has the first and last vertex the same
        root_vertex: Tour starts and ends at this vertex
        step_size: Gap between two vertices in the tour when trying to extend the tour
        path_depth_limit: Length of the path to explore in order to extend the tour
        logging_level: Verbosity of logging.

    Returns:
        Tour that has prize above the quota
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edge_list = list(graph.edges())
    extended_tour: VertexList = extension_bind(
        edge_list,
        tour,
        cost_dict,
        prize_dict,
        root_vertex,
        step_size,
        path_depth_limit,
        logging_level,
    )
    return extended_tour


def extension_until_prize_feasible(
    graph: nx.Graph,
    tour: VertexList,
    root_vertex: int,
    quota: int,
    step_size: int = 1,
    path_depth_limit: int = 2,
    logging_level: int = logging.INFO,
) -> VertexList:
    """Increase the prize of the tour by selecting vertices according to their unitary loss
    until the total prize of the tour is at least the quota (or until the tour cannot be
    extended any further).

    Args:
        graph: Undirected input graph
        tour: Tour that has the first and last vertex the same
        root_vertex: Tour starts and ends at this vertex
        quota: Lower bound on total prize of tour
        step_size: Gap between two vertices in the tour when trying to extend the tour
        path_depth_limit: Length of the path to explore in order to extend the tour
        logging_level: Verbosity of logging.

    Returns:
        Tour that has prize above the quota
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edge_list = list(graph.edges())
    extended_tour: VertexList = extension_until_prize_feasible_bind(
        edge_list,
        tour,
        cost_dict,
        prize_dict,
        root_vertex,
        quota,
        step_size,
        path_depth_limit,
        logging_level,
    )
    return extended_tour


def path_extension_collapse(
    graph: nx.Graph,
    tour: VertexList,
    root_vertex: Vertex,
    quota: int,
    collapse_shortest_paths: bool = False,
    path_depth_limit: int = 2,
    step_size: int = 1,
) -> VertexList:
    """Run the path extension & collapse heuristic

    Args:
        graph: Undirected input graph
        tour: Tour that has the first and last vertex the same
        root_vertex: Tour starts and ends at this vertex
        quota: Lower bound on total prize of tour
        collapse_shortest_paths: If true, collapse the tour by finding shortest paths
        path_depth_limit: Length of the path to explore in order to extend the tour
        step_size: Gap between two vertices in the tour when trying to extend the tour

    Returns:
        Tour that (hopefully) has prize above the quota
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edge_list = list(graph.edges())
    return path_extension_collapse_bind(
        edge_list,
        tour,
        cost_dict,
        prize_dict,
        root_vertex,
        quota,
        collapse_shortest_paths,
        path_depth_limit,
        step_size,
    )


def tour_from_vertex_disjoint_paths(vertex_disjoint_paths: DisjointPaths) -> VertexList:
    """Get a tour from a pair of vertex disjoint paths in an undirected graph

    Args:
        vertex_disjoint_paths: Vertex disjoint paths. Start vertex is the same for both
            disjoint paths. End vertex is the same for both paths.

    Returns:
        Tour formed by reversing the second disjoint paths then
        appending it to the first disjoint path

    Raises:
        ValueError: If one of the vertex disjoint paths is empty or
            if the first (last) vertices of both paths are not the same
    """
    first_path = vertex_disjoint_paths[0].copy()
    second_path = vertex_disjoint_paths[1].copy()
    if len(first_path) <= 1:
        raise ValueError(
            "The first vertex disjoint path is empty or contains only one vertex"
        )
    if len(second_path) <= 1:
        raise ValueError(
            "The second disjoint path is empty is empty or contains only one vertex"
        )
    if first_path[0] != second_path[0]:
        raise ValueError("The first vertex of both paths are not the same")
    if first_path[len(first_path) - 1] != second_path[len(second_path) - 1]:
        raise ValueError(
            "The last vertex of both vertex disjoint paths are not the same"
        )
    second_path.reverse()
    second_path = second_path[1:]
    first_path.extend(second_path)
    return first_path


def suurballes_heuristic(
    prize_map: VertexFunction,
    quota: int,
    vertex_disjoint_cost_map: VertexFunction,
    vertex_disjoint_paths_map: Mapping[Vertex, DisjointPaths],
) -> VertexList:
    """Heuristic for Prize-collecting TSP that uses Suurballe's algorithm

    Args:
        prize_map: Mapping from vertex to the prize of the vertex
        quota: The minimum amount of prize the tour must collect
        vertex_disjoint_cost_map: Mapping from vertex to cost of the least-cost pair
            of vertex-disjoint paths from the root vertex
        vertex_disjoint_paths_map: Mapping from vertex to disjoint paths

    Returns:
        The least-cost prize feasible tour obtained from a pair of vertex disjoint paths
    """
    least_cost: int = sys.maxsize
    best_tour: VertexList = []
    for vertex, disjoint_paths in vertex_disjoint_paths_map.items():
        tour = tour_from_vertex_disjoint_paths(disjoint_paths)
        prize_of_tour = total_prize_of_tour(prize_map, tour)
        cost_of_tour = vertex_disjoint_cost_map[vertex]
        if (
            cost_of_tour < least_cost
            and prize_of_tour >= quota
            and len(set(tour)) == len(tour) - 1
        ):
            best_tour = tour
            least_cost = cost_of_tour
    return best_tour


def random_tour_complete_graph(
    graph: nx.Graph, root_vertex: Vertex, quota: int, seed: int = 0
) -> VertexList:
    """Return a random tour that is prize-feasible

    Args:
        graph: Undirected complete graph with prize and cost attributes
        root_vertex: Start and end vertex of the tour
        quota: Amount of prize required to be collected
        seed: Set the seed for randomly choosing vertices

    Returns:
        Tour of the graph
    """
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    prize = prize_dict[root_vertex]
    random.seed(seed)
    tour = [root_vertex]
    vertices_not_in_tour = list(graph.nodes())  # quickly check membership
    vertices_not_in_tour.remove(root_vertex)
    while len(vertices_not_in_tour) > 0 and prize < quota:
        # choose a vertex that has not yet been added to the tour
        vertex = random.choice(vertices_not_in_tour)
        prize += prize_dict[vertex]
        tour.append(vertex)
        vertices_not_in_tour.remove(vertex)

    # add the root vertex to the end of the tour
    tour.append(root_vertex)
    return tour


def random_tour_from_disjoint_paths_map(
    disjoint_paths_map: Mapping[Vertex, DisjointPaths],
    root_vertex: Vertex,
    seed: int = 0,
) -> VertexList:
    """Obtain a tour from a randomly chosen pair of vertex disjoint paths

    Args:
        disjoint_paths_map: Vertices are keys, values are pairs of vertex-disjoint
            paths
        root_vertex: The vertex the tour starts and ends at
        seed: Initialise the seed for fixing randomness

    Returns:
        A random tour generated from one of the vertex disjoint paths

    Raises:
        ValueError: If no sufficient disjoint paths are given
    """
    random.seed(seed)

    vertex_choices = list(disjoint_paths_map.keys())
    if root_vertex in vertex_choices:
        vertex_choices.remove(root_vertex)
    if len(vertex_choices) == 0:
        error_message = "No vertex disjoint paths to create a cycle from "
        error_message += "(excluding between the root and itself)"
        raise ValueError(error_message)

    keep_repeating = True
    vertex = random.choice(vertex_choices)
    while len(vertex_choices) >= 0 and keep_repeating:
        first_path = disjoint_paths_map[vertex][0]
        second_path = disjoint_paths_map[vertex][1]
        keep_repeating = len(first_path) + len(second_path) < 5
        vertex_choices.remove(vertex)
        vertex = random.choice(vertex_choices)
    return tour_from_vertex_disjoint_paths(disjoint_paths_map[vertex])


def find_cycle_from_bfs(G: nx.Graph, root_vertex: Vertex) -> VertexList:
    """Find a simple cycle starting and ending at the root vertex
    from a breadth first traversal of the graph

    Args:
        G: Undirected input graph
        root_vertex: Vertex to start the BFS traversal from

    Returns:
        List of vertices in the simple cycle, including the root vertex
    """
    tree = nx.bfs_tree(G, root_vertex)
    color = 0
    queue = []
    vertex_color = {}
    for successor in tree.successors(root_vertex):
        vertex_color[successor] = color
        color += 1
        queue.append(successor)

    cycle: VertexList = []
    while queue:
        vertex = queue.pop(0)
        branch_neighbors = set()
        color = vertex_color[vertex]
        parent = list(tree.predecessors(vertex))[0]
        branch_neighbors.add(parent)
        for child in tree.successors(vertex):
            vertex_color[child] = color
            branch_neighbors.add(child)
            queue.append(child)
        for neighbor in set(G.neighbors(vertex)) - branch_neighbors:
            if vertex_color[neighbor] != color:
                # then we have found a simple cycle containing the root...
                # get the part from vertex to the root
                path_from_root_to_vertex: VertexList = path_to_vertex_in_tree(
                    tree, vertex
                )
                # get the path from the neighbor to the root
                path_from_root_to_neighbor: VertexList = path_to_vertex_in_tree(
                    tree, neighbor
                )
                # create a cycle containing the vertex, root and neighbor
                path_from_root_to_neighbor.reverse()
                path_from_root_to_vertex.extend(path_from_root_to_neighbor)
                cycle = path_from_root_to_vertex
                while queue:
                    queue.pop()
                break
    return cycle


def path_to_vertex_in_tree(T: nx.DiGraph, target: Vertex) -> VertexList:
    """Find a path from the root vertex to the target vertex in a tree"""
    path_from_root_to_vertex: VertexList = []
    current = target
    while len(list(T.predecessors(current))) == 1:
        path_from_root_to_vertex.insert(0, current)
        current = list(T.predecessors(current))[0]
    path_from_root_to_vertex.insert(0, current)
    return path_from_root_to_vertex
