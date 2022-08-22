"""Heuristics for the Prize-Collecting TSP"""

import random
import sys
from typing import Mapping
import networkx as nx
from tspwplib import (
    DisjointPaths,
    Vertex,
    VertexFunction,
    VertexFunctionName,
    VertexList,
    total_prize_of_tour,
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


def suurballes_tour_initialization(
    prize_map: VertexFunction,
    quota: int,
    vertex_disjoint_cost_map: VertexFunction,
    vertex_disjoint_paths_map: Mapping[Vertex, DisjointPaths],
) -> VertexList:
    """Find a tour to initialize the (Path) Extension and Collapse heuristic

    Args:
        prize_map: Mapping from vertex to the prize of the vertex
        quota: The minimum amount of prize the tour must collect
        vertex_disjoint_cost_map: Mapping from vertex to cost of the least-cost pair
            of vertex-disjoint paths from the root vertex
        vertex_disjoint_paths_map: Mapping from vertex to disjoint paths

    Returns:
        The least-cost prize feasible tour obtained from a pair of vertex disjoint paths

    Notes:
        Behaves the same as Suurballe's heuristic if there exists a prize-feasible pair
        of least-cost vertex disjoint paths in the graph.
        Otherwise, return the tour that maximizes the total prize of pairs of least-cost
        vertex disjoint paths from the root vertex.
    """
    # first run Suurballe's heuristic: takes O(n^2) time
    tour = suurballes_heuristic(
        prize_map, quota, vertex_disjoint_cost_map, vertex_disjoint_paths_map
    )
    if tour:
        return tour

    # if no prize feasible tour is returned, then return the tour that maximizes the prize
    biggest_prize = 0
    biggest_tour = []
    for disjoint_paths in vertex_disjoint_paths_map.values():
        tour = tour_from_vertex_disjoint_paths(disjoint_paths)
        prize_of_tour = total_prize_of_tour(prize_map, tour)
        if prize_of_tour > biggest_prize:
            biggest_tour = tour
            biggest_prize = prize_of_tour
    return biggest_tour


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
    while len(vertices_not_in_tour) > 0 and (prize < quota or len(tour) < 3):
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
