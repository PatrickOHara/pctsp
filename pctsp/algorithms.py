"""Exact algorithms for the Prize collecting TSP with a branch and cut solver."""

import logging
from pathlib import Path
import networkx as nx
from pyscipopt import Model
from tspwplib import (
    EdgeFunctionName,
    Vertex,
    VertexFunction,
    VertexFunctionName,
    EdgeList,
    is_pctsp_yes_instance,
)
from .constants import (
    FOUR_HOURS,
)

# pylint: disable=import-error
from .libpypctsp import solve_pctsp_bind

# pylint: enable=import-error


def solve_pctsp(
    model: Model,
    graph: nx.Graph,
    heuristic_edges: EdgeList,
    quota: int,
    root_vertex: Vertex,
    branching_max_depth: int = -1,
    branching_strategy: int = 0,
    cost_cover_disjoint_paths: bool = False,
    cost_cover_shortest_path: bool = False,
    cycle_cover: bool = False,
    disjoint_paths_cost: VertexFunction = None,
    logging_level: int = logging.INFO,
    name: str = "pctsp",
    solver_dir: Path = Path("."),
    sec_disjoint_tour: bool = True,
    sec_lp_gap_improvement_threshold: float = 0.001,
    sec_maxflow_mincut: bool = True,
    sec_max_tailing_off_iterations: int = -1,
    sec_sepafreq: int = 1,
    time_limit: float = FOUR_HOURS,
) -> EdgeList:
    """Solve Prize-collecting TSP with branch and cut

    Args:
        model: Empty SCIP model
        graph: Undirected input graph with edge costs and vertex prizes
        heuristic_edges: Edges in a heuristic solution
        quota: The minimum prize the tour must collect
        root_vertex: The tour must start and end at the root vertex
        cost_cover_disjoint_paths: True if disjoint paths cost cover inequality is used
        cost_cover_shortest_path: True if shortest paths cost cover inequality is used
        cycle_cover: True to add cycle cover inequalities
        logging_level: How verbose should the logging be, e.g. logging.DEBUG?
        name: Name of the problem instance
        solver_dir: Directory to store logs and metrics
        sec_disjoint_tour: True if subtour elimination constraints using disjoint tours are used
        sec_maxflow_mincut: True if using the maxflow mincut SEC separation algorithm
        time_limit: Stop searching after this many seconds

    Returns:
        Edge list of the optimal tour
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edges = list(graph.edges())
    solver_dir.mkdir(exist_ok=True, parents=False)
    initial_yes_instance = []
    if heuristic_edges and is_pctsp_yes_instance(
        graph, quota, root_vertex, heuristic_edges
    ):
        initial_yes_instance = heuristic_edges

    if not disjoint_paths_cost:
        cost_cover_disjoint_paths = False
        disjoint_paths_cost = {}

    return solve_pctsp_bind(
        model,
        edges,
        initial_yes_instance,
        cost_dict,
        prize_dict,
        quota,
        root_vertex,
        branching_max_depth,
        branching_strategy,
        cost_cover_disjoint_paths,
        cost_cover_shortest_path,
        cycle_cover,
        disjoint_paths_cost,
        logging_level,
        name,
        sec_disjoint_tour,
        sec_lp_gap_improvement_threshold,
        sec_maxflow_mincut,
        sec_max_tailing_off_iterations,
        sec_sepafreq,
        solver_dir,
        time_limit,
    )
