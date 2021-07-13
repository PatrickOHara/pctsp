"""Exact algorithms for the Prize collecting TSP with a branch and cut solver."""

import logging
from pathlib import Path
from typing import Optional
import networkx as nx
from tspwplib import EdgeFunctionName, Vertex, VertexFunctionName, EdgeList
from .constants import FOUR_HOURS

# pylint: disable=import-error
from .libpypctsp import pctsp_branch_and_cut_bind

# pylint: enable=import-error


def pctsp_disjoint_tours_relaxation(
    graph: nx.Graph,
    quota: int,
    root_vertex: Vertex,
    log_file: Optional[Path] = None,
    logging_level: int = logging.INFO,
    time_limit: float = FOUR_HOURS,
) -> EdgeList:
    """Find the relaxation of PCTSP where no subtour elimination constraints are added"""
    pctsp_branch_and_cut(
        graph,
        quota,
        root_vertex,
        False,
        False,
        False,
        log_file,
        logging_level,
        False,
        0,
        False,
        0,
        time_limit,
    )


# pylint: disable=too-many-arguments
def pctsp_branch_and_cut(
    graph: nx.Graph,
    quota: int,
    root_vertex: Vertex,
    cost_cover_disjoint_paths: bool = False,
    cost_cover_shortest_path: bool = False,
    cost_cover_steiner_tree: bool = False,
    log_file: Optional[Path] = None,
    logging_level: int = logging.INFO,
    sec_disjoint_tour: bool = True,
    sec_disjoint_tour_freq: int = 1,
    sec_maxflow_mincut: bool = True,
    sec_maxflow_mincut_freq: int = 1,
    time_limit: float = FOUR_HOURS,
) -> EdgeList:
    """Branch and cut algorithm for the prize collecting travelling salesman problem

    Args:
        graph: Undirected input graph with edge costs and vertex prizes
        quota: The minimum prize the tour must collect
        root_vertex: The tour must start and end at the root vertex
        cost_cover_disjoint_paths: True if disjoint paths cost cover inequality is used
        cost_cover_shortest_paths: True if shortest paths cost cover inequality is used
        cost_cover_steiner_tree: True if Steiner tree cost cover inequality is used
        log_file: Optional path to store the logs of the algorithm
        logging_level: How verbose should the logging be, e.g. logging.DEBUG?
        sec_disjoint_tour: True if subtour elimination constraints using disjoint tours are used
        sec_disjoint_tour_freq: How often should disjoint tour SECs be added as cutting planes
        sec_maxflow_mincut: True if using the maxflow mincut SEC separation algorithm
        sec_maxflow_mincut_freq: How frequently to add maxflow mincut SECs.
        time_limit: Stop searching after this many seconds

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
        cost_cover_disjoint_paths,
        cost_cover_shortest_path,
        cost_cover_steiner_tree,
        str_log_file,
        logging_level,
        sec_disjoint_tour,
        sec_disjoint_tour_freq,
        sec_maxflow_mincut,
        sec_maxflow_mincut_freq,
        time_limit,
    )
    return optimal_edges
