"""Heuristics for the Prize-Collecting TSP"""

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
    total_prize,
)

# pylint: disable=import-error
from .libpypctsp import (
    collapse_bind,
    extend_bind,
    extend_until_prize_feasible_bind,
)

# pylint: enable=import-error


def collapse(
    graph: nx.Graph, tour: VertexList, quota: int, root_vertex: Vertex
) -> VertexList:
    """Collapse the tour by finding shortcuts

    Args:
        graph: Undirected input graph with edge costs and vertex prizes
        tour: Tour that has the first and last vertex the same
        quota: The minimum prize the tour must collect
        root_vertex: The tour must start and end at the root vertex

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
    )
    return collapsed_tour


def extend(graph: nx.Graph, tour: VertexList) -> VertexList:
    """Add vertices to a tour according to their unitary gain

    Args:
        graph: Undirected input graph
        tour: Tour that has the first and last vertex the same

    Returns:
        Tour that has prize above the quota
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edge_list = list(graph.edges())
    extended_tour: VertexList = extend_bind(
        edge_list,
        tour,
        cost_dict,
        prize_dict,
    )
    return extended_tour


def extend_until_prize_feasible(graph: nx.Graph, tour: VertexList, quota: int):
    """Given a tour of the graph, add vertices until it is prize-feasible

    Args:
        graph: Undirected input graph
        tour: Tour that has the first and last vertex the same
        quota: Prize threshold of the tour

    Returns:
        Tour that has prize above the quota
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edge_list = list(graph.edges())
    extended_tour: VertexList = extend_until_prize_feasible_bind(
        edge_list,
        tour,
        cost_dict,
        prize_dict,
        quota,
    )
    return extended_tour


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