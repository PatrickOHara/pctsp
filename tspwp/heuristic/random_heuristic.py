"""Heuristics that have a random component"""

from typing import Mapping
import random
import graph_tool as gt
from tspwplib import DisjointPaths, Vertex, VertexList
from .suurballes_heuristic import tour_from_vertex_disjoint_paths


def random_tour_complete_graph(
    graph: gt.Graph, root_vertex: Vertex, quota: int, seed: int = 0
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
    prize = graph.vp.prize[root_vertex]
    random.seed(seed)
    tour = [root_vertex]
    vertices_not_in_tour = set(graph.vertices())  # quickly check membership
    vertices_not_in_tour.remove(root_vertex)
    while len(vertices_not_in_tour) > 0 and prize < quota:
        # choose a vertex that has not yet been added to the tour
        j = random.randint(0, graph.num_vertices() - len(tour) - 1)
        vertex = graph.vertex(j)
        prize += graph.vp.prize[vertex]
        tour.append(vertex)

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
