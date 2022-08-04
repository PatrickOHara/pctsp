"""The Extension and Collapse heuristic as described in the paper:

M. Dell’Amico, F. Maffioli and A. Sciomachen. 1998.
A Lagrangian heuristic for the Prize Collecting Travelling Salesman Problem
Annals of Operations Research 81, pages 289-305.
"""

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
    extension_unitary_gain_bind,
    extension_until_prize_feasible_bind,
    path_extension_bind,
)

# pylint: enable=import-error


def collapse(
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
        False,
        logging_level,
    )
    return collapsed_tour


def extension_unitary_gain_collapse(
    graph: nx.Graph,
    tour: VertexList,
    quota: int,
    root_vertex: Vertex,
    logging_level: int = logging.INFO,
) -> VertexList:
    """Implementation of Extension & Collapse with unitary gain as described in
    the paper by Dell'Amico et al.

    Args:
        graph: Undirected input graph with edge costs and vertex prizes
        tour: Tour that has the first and last vertex the same
        quota: The minimum prize the tour must collect
        root_vertex: The tour must start and end at the root vertex
        logging_level: Verbosity of logging.

    Returns:
        A collapsed, prize-feasible tour that has at most the same cost as the input tour
    """
    extended_tour = extension_unitary_gain(graph, tour, logging_level=logging_level)
    return collapse(
        graph, extended_tour, quota, root_vertex, logging_level=logging_level
    )


def extension_unitary_gain(
    graph: nx.Graph,
    tour: VertexList,
    logging_level: int = logging.INFO,
) -> VertexList:
    """Implementation of the Extension algorithm with Unitary Gain

    Args:
        graph: Undirected input graph
        tour: Tour that has the first and last vertex the same
        logging_level: Verbosity of logging.

    Returns:
        Tour that has prize greater than or equal to the input tour
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edge_list = list(graph.edges())
    extended_tour: VertexList = extension_unitary_gain_bind(
        edge_list,
        tour,
        cost_dict,
        prize_dict,
        logging_level,
    )
    return extended_tour


def extension_unitary_loss(
    graph: nx.Graph,
    tour: VertexList,
    root_vertex: int,
    logging_level: int = logging.INFO,
) -> VertexList:
    """Extension algorithm with unitary loss. No path extension.

    Args:
        graph: Undirected input graph
        tour: Tour that has the first and last vertex the same
        root_vertex: Tour starts and ends at this vertex
        logging_level: Verbosity of logging.

    Returns:
        Tour that has prize above the quota
    """
    step_size: int = 1
    path_depth_limit: int = 2
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edge_list = list(graph.edges())
    extended_tour: VertexList = path_extension_bind(
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
    quota: int,
) -> VertexList:
    """Implementation of Extension & Collapse with unitary gain as described in
    the paper by Dell'Amico et al.
    Extends the input tour until the total prize is at least the quota.

    Args:
        graph: Undirected input graph with edge costs and vertex prizes
        tour: Tour that has the first and last vertex the same
        quota: The minimum prize the tour must collect

    Returns:
        A collapsed, prize-feasible tour that has at most the same cost as the input tour
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edge_list = list(graph.edges())
    return extension_until_prize_feasible_bind(
        edge_list,
        tour,
        cost_dict,
        prize_dict,
        quota,
    )
