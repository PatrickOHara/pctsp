"""The path extension & collapse algorithm"""

import logging
import networkx as nx
from tspwplib import (
    EdgeFunctionName,
    Vertex,
    VertexFunctionName,
    VertexList,
)

# pylint: disable=import-error
from ..libpypctsp import (
    collapse_bind,
    path_extension_until_prize_feasible_bind,
    path_extension_collapse_bind,
)


def path_collapse(
    graph: nx.Graph,
    tour: VertexList,
    quota: int,
    root_vertex: Vertex,
    logging_level: int = logging.INFO,
) -> VertexList:
    """Collapse the tour by finding shortcuts

    Args:
        graph: Undirected input graph with edge costs and vertex prizes
        tour: Tour that has the first and last vertex the same
        quota: The minimum prize the tour must collect
        root_vertex: The tour must start and end at the root vertex
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
        True,
        logging_level,
    )
    return collapsed_tour


def path_extension_collapse(
    graph: nx.Graph,
    tour: VertexList,
    root_vertex: Vertex,
    quota: int,
    collapse_shortest_paths: bool = False,
    path_depth_limit: int = 2,
    step_size: int = 1,
    logging_level: int = logging.INFO,
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
        logging_level: Verbosity of logging

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
        logging_level,
    )


def path_extension_until_prize_feasible(
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
    extended_tour: VertexList = path_extension_until_prize_feasible_bind(
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
