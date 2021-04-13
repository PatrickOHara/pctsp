"""Heuristics for the Prize-Collecting TSP"""

import networkx as nx
from tspwplib import EdgeFunctionName, VertexFunctionName, VertexList

# pylint: disable=import-error
from .libpypctsp import (
    # collapse_bind,
    extend_bind,
    # extend_until_prize_feasible_bind,
)

# pylint: enable=import-error


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
