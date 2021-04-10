"""Heuristic for Prize-collecting TSP that uses Suurballe's algorithm"""

import sys
from typing import Mapping
from tspwplib import (
    DisjointPaths,
    Vertex,
    VertexFunction,
    VertexList,
    total_prize,
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
        prize_of_tour = total_prize(prize_map, tour)
        cost_of_tour = vertex_disjoint_cost_map[vertex]
        if (
            cost_of_tour < least_cost
            and prize_of_tour >= quota
            and len(set(tour)) == len(tour) - 1
        ):
            best_tour = tour
            least_cost = cost_of_tour
    return best_tour
