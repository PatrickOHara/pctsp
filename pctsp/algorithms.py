"""Exact algorithms for the Prize collecting TSP with a branch and cut solver."""

import logging
from pathlib import Path
from typing import Optional
import networkx as nx
from tspwplib import EdgeFunctionName, Vertex, VertexFunctionName, EdgeList

# pylint: disable=import-error
from .libpypctsp import pctsp_branch_and_cut_bind

# pylint: enable=import-error


def pctsp_branch_and_cut(
    graph: nx.Graph,
    quota: int,
    root_vertex: Vertex,
    log_file: Optional[Path] = None,
    logging_level: int = logging.INFO,
) -> EdgeList:
    """Branch and cut algorithm for the prize collecting travelling salesman problem

    Args:
        graph: Undirected input graph with edge costs and vertex prizes
        quota: The minimum prize the tour must collect
        root_vertex: The tour must start and end at the root vertex
        log_file: Optional path to store the logs of the algorithm
        logging_level: How verbose should the logging be, e.g. logging.DEBUG?

    Returns:
        Edge list of the optimal tour
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edges = list(graph.edges())
    if log_file:
        str_log_file = str(log_file)
    else:
        str_log_file = ""
    optimal_edges: EdgeList = pctsp_branch_and_cut_bind(
        edges,
        prize_dict,
        cost_dict,
        quota,
        root_vertex,
        str_log_file,
        logging_level,
    )
    return optimal_edges
