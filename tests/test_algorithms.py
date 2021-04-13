"""Tests for exact algorithms for PCTSP"""

import networkx as nx
from pctsp import pctsp_branch_and_cut
from tspwplib import edge_list_from_walk, total_prize, total_cost_networkx


def test_pctsp_on_suurballes_graph(suurballes_undirected_graph, suurballe_source):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 6
    edge_list = pctsp_branch_and_cut(
        suurballes_undirected_graph, quota, suurballe_source
    )
    assert len(edge_list) > 0
    # assert (
    #     total_prize(
    #         nx.get_node_attributes(suurballes_undirected_graph, "prize"),
    #         optimal_tour,
    #     )
    #     >= quota
    # )
    # assert total_cost_networkx(suurballes_undirected_graph, optimal_tour) > 0


def test_pctsp_on_tspwplib(tspwplib_graph, root):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 6
    edge_list = pctsp_branch_and_cut(tspwplib_graph, quota, root)
    assert len(edge_list) > 0

    # prize_dict = nx.get_node_attributes(tspwplib_graph, "prize")
    # assert total_prize(prize_dict, optimal_tour) >= quota
    # assert total_cost_networkx(tspwplib_graph, optimal_tour) > 0
