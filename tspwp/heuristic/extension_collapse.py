"""Heuristics for PCTSP"""

import graph_tool as gt
from tspwplib import EdgeFunctionName, Vertex, VertexFunctionName, VertexList

# pylint: disable=import-error
from .libheuristic import (
    collapse_bind,
    extend_bind,
    extend_until_prize_feasible_bind,
)

# pylint: enable=import-error


def collapse(
    graph: gt.Graph, tour: VertexList, quota: int, root_vertex: Vertex
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
    # pylint: disable=protected-access
    cost_map = graph.edge_properties[EdgeFunctionName.cost.value]._get_any()
    prize_map = graph.vertex_properties[VertexFunctionName.prize.value]._get_any()
    collapsed_tour: VertexList = collapse_bind(
        graph._Graph__graph,
        tour,
        cost_map,
        prize_map,
        quota,
        root_vertex,
    )
    return collapsed_tour


def extend(graph: gt.Graph, tour: VertexList) -> VertexList:
    """Add vertices to a tour according to their unitary gain

    Args:
        graph: Undirected input graph
        tour: Tour that has the first and last vertex the same

    Returns:
        Tour that has prize above the quota
    """
    # pylint: disable=protected-access
    cost_map = graph.edge_properties[EdgeFunctionName.cost.value]._get_any()
    prize_map = graph.vertex_properties[VertexFunctionName.prize.value]._get_any()
    extended_tour: VertexList = extend_bind(
        graph._Graph__graph,
        tour,
        cost_map,
        prize_map,
    )
    return extended_tour


def extend_until_prize_feasible(graph: gt.Graph, tour: VertexList, quota: int):
    """Given a tour of the graph, add vertices until it is prize-feasible

    Args:
        graph: Undirected input graph
        tour: Tour that has the first and last vertex the same
        quota: Prize threshold of the tour

    Returns:
        Tour that has prize above the quota
    """
    # pylint: disable=protected-access
    cost_map = graph.edge_properties[EdgeFunctionName.cost.value]._get_any()
    prize_map = graph.vertex_properties[VertexFunctionName.prize.value]._get_any()
    extended_tour: VertexList = extend_until_prize_feasible_bind(
        graph._Graph__graph,
        tour,
        cost_map,
        prize_map,
        quota,
    )
    return extended_tour
