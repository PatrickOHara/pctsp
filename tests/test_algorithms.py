"""Tests for exact algorithms for PCTSP"""

import networkx as nx
from pctsp import pctsp_branch_and_cut
from tspwplib import (
    edge_list_from_walk,
    order_edge_list,
    total_prize,
    total_cost_networkx,
    is_pctsp_yes_instance,
    vertex_set_from_edge_list,
    walk_from_edge_list,
)


def test_pctsp_on_suurballes_graph(suurballes_undirected_graph, root):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 6
    edge_list = pctsp_branch_and_cut(suurballes_undirected_graph, quota, root)
    assert len(edge_list) > 0
    ordered_edges = order_edge_list(edge_list)
    optimal_tour = walk_from_edge_list(ordered_edges)
    assert is_pctsp_yes_instance(
        suurballes_undirected_graph, quota, root, ordered_edges
    )
    assert total_cost_networkx(suurballes_undirected_graph, optimal_tour) == 20


def test_pctsp_on_tspwplib(tspwplib_graph, root):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 20
    edge_list = pctsp_branch_and_cut(tspwplib_graph, quota, root)
    ordered_edges = order_edge_list(edge_list)
    assert len(edge_list) > 0
    optimal_tour = walk_from_edge_list(ordered_edges)
    assert is_pctsp_yes_instance(tspwplib_graph, quota, root, ordered_edges)
    assert total_cost_networkx(tspwplib_graph, optimal_tour) > 0
