"""Tests for exact algorithms for PCTSP"""

from tspwplib import (
    edge_list_from_walk,
    order_edge_list,
    reorder_edge_list_from_root,
    total_cost_networkx,
    is_pctsp_yes_instance,
    walk_from_edge_list,
)
from pctsp import pctsp_branch_and_cut, random_tour_complete_graph


def test_pctsp_on_suurballes_graph(suurballes_undirected_graph, root):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 6
    edge_list = pctsp_branch_and_cut(suurballes_undirected_graph, quota, root)
    assert len(edge_list) > 0
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    optimal_tour = walk_from_edge_list(ordered_edges)
    assert is_pctsp_yes_instance(
        suurballes_undirected_graph, quota, root, ordered_edges
    )
    assert total_cost_networkx(suurballes_undirected_graph, optimal_tour) == 20


def test_pctsp_on_tspwplib(tspwplib_graph, root):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 30
    edge_list = pctsp_branch_and_cut(tspwplib_graph, quota, root)
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    assert len(edge_list) > 0
    optimal_tour = walk_from_edge_list(ordered_edges)
    assert is_pctsp_yes_instance(tspwplib_graph, quota, root, ordered_edges)
    assert total_cost_networkx(tspwplib_graph, optimal_tour) > 0


def test_pctsp_with_heuristic(tspwplib_graph, root):
    """Test adding an initial solution to solver"""
    quota = 30
    tour = random_tour_complete_graph(tspwplib_graph, root, quota)
    edge_list = edge_list_from_walk(tour)
    print(edge_list)
    if len(edge_list) > 2:
        pctsp_branch_and_cut(tspwplib_graph, quota, root, initial_solution=edge_list)
    # assert False
