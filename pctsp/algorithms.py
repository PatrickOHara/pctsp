"""Exact algorithms for the Prize collecting TSP with a branch and cut solver."""

import networkx as nx
from tspwplib import EdgeFunctionName, Vertex, VertexFunctionName, EdgeList

# pylint: disable=import-error
from .libpypctsp import pctsp_branch_and_cut_bind

# pylint: enable=import-error


def pctsp_branch_and_cut(graph: nx.Graph, quota: int, root_vertex: Vertex) -> EdgeList:
    """Branch and cut algorithm for the prize collecting travelling salesman problem

    Args:
        graph: Undirected input graph with edge costs and vertex prizes
        quota: The minimum prize the tour must collect
        root_vertex: The tour must start and end at the root vertex

    Returns:
        A collapsed, prize-feasible tour that has at most the same cost as the input tour
    """
    cost_dict = nx.get_edge_attributes(graph, EdgeFunctionName.cost.value)
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    edges = list(graph.edges())
    optimal_edges: EdgeList = pctsp_branch_and_cut_bind(
        edges,
        prize_dict,
        cost_dict,
        quota,
        root_vertex,
    )
    return optimal_edges
