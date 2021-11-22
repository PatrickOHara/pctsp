"""Test a boost graph is loaded in cpp given a python edge list"""

import networkx as nx
from pctsp import graph_from_edge_list


def test_graph_from_edge_list(suurballes_undirected_graph):
    """Test edge list creates a graph"""
    cost_map = nx.get_edge_attributes(suurballes_undirected_graph, "cost")
    prize_map = nx.get_node_attributes(suurballes_undirected_graph, "prize")
    edge_list = list(suurballes_undirected_graph.edges())
    assert isinstance(cost_map, dict)
    assert isinstance(prize_map, dict)
    assert isinstance(edge_list, list)
    assert len(edge_list) == len(cost_map)
    assert len(edge_list) > 0
    assert len(prize_map) > 0
    assert graph_from_edge_list(edge_list, prize_map, cost_map)


def test_grid8(grid8):
    """Read grid8 graph from dot file"""
    assert grid8.number_of_nodes() == 8
    assert grid8.number_of_edges() == 10
    assert grid8[1][4]["cost"] == 5
