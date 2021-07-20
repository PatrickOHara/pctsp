"""Exact algorithms for the Prize collecting TSP with a branch and cut solver."""

import logging
from pathlib import Path
from typing import Optional
import networkx as nx
from tspwplib import (
    EdgeFunctionName,
    Vertex,
    VertexFunctionName,
    EdgeList,
    is_pctsp_yes_instance,
)
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
    raise NotImplementedError()
    # pctsp_branch_and_cut(
    #     graph,
    #     quota,
    #     root_vertex,
    #     cost_cover_disjoint_paths=False,
    #     cost_cover_shortest_path=False,
    #     cost_cover_steiner_tree=False,
    #     initial_solution=None,
    #     log_file=log_file,
    #     logging_level=logging_level,
    #     sec_disjoint_tour=False,
    #     sec_disjoint_tour_freq=0,
    #     sec_maxflow_mincut=False,
    #     sec_maxflow_mincut_freq=0,
    #     time_limit=time_limit,
    # )


# pylint: disable=too-many-arguments
def pctsp_branch_and_cut(
    graph: nx.Graph,
    quota: int,
    root_vertex: Vertex,
    cost_cover_disjoint_paths: bool = False,
    cost_cover_shortest_path: bool = False,
    cost_cover_steiner_tree: bool = False,
    initial_solution: Optional[EdgeList] = None,
    log_boost_filename: str = "boost_logs.txt",
    log_scip_filename: str = "scip_logs.txt",
    logging_level: int = logging.INFO,
    metrics_filename: str = "scip_metrics.csv",
    name: str = "pctsp",
    output_dir: Optional[Path] = None,
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
        initial_solution: Edges of a feasible solution found by a heuristic
        log_boost_filename: Name of file to store the logs of algorithms
        log_scip_filename: Name of ile to store logs of the solver
        logging_level: How verbose should the logging be, e.g. logging.DEBUG?
        name: Name of the problem instance
        output_dir: Directory to store logs and metrics
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
    if not output_dir:
        output_dir = Path(".")
    output_dir.mkdir(exist_ok=True, parents=False)
    metrics_filepath = output_dir / metrics_filename
    log_boost_filepath = output_dir / log_boost_filename
    log_scip_filepath = output_dir / log_scip_filename
    print("Log scip filepath python:", str(log_scip_filepath))
    initial_yes_instance = list()
    if initial_solution and is_pctsp_yes_instance(
        graph, quota, root_vertex, initial_solution
    ):
        initial_yes_instance = initial_solution

    optimal_edges: EdgeList = pctsp_branch_and_cut_bind(
        edges,
        prize_dict,
        cost_dict,
        quota,
        root_vertex,
        cost_cover_disjoint_paths,
        cost_cover_shortest_path,
        cost_cover_steiner_tree,
        initial_yes_instance,
        str(log_boost_filepath),
        str(log_scip_filepath),
        logging_level,
        str(metrics_filepath),
        name,
        sec_disjoint_tour,
        sec_disjoint_tour_freq,
        sec_maxflow_mincut,
        sec_maxflow_mincut_freq,
        time_limit,
    )
    return optimal_edges
