"""Tests for preprocessing a networkx graph"""

import networkx as nx
from pctsp import (
    remove_leaves,
    remove_components_disconnected_from_vertex,
)


def test_remove_leaves(disconnected_graph):
    """Test all leaves are removed"""
    # test for non complete graph
    graph = remove_leaves(disconnected_graph)
    assert graph.number_of_nodes() == 3
    assert graph.number_of_edges() == 3

    # test for complete graph
    complete = nx.complete_graph(5)
    graph = remove_leaves(complete)
    assert complete.number_of_nodes() == graph.number_of_nodes()
    assert complete.number_of_edges() == graph.number_of_edges()


def test_remove_components(disconnected_graph):
    """Test disconnected components are removed from the graph"""
    graph = remove_components_disconnected_from_vertex(disconnected_graph, 0)
    assert graph.number_of_nodes() == 4
    assert graph.number_of_edges() == 4

    # test for complete graph
    complete = nx.complete_graph(5)
    graph = remove_components_disconnected_from_vertex(complete, 0)
    assert complete.number_of_nodes() == graph.number_of_nodes()
    assert complete.number_of_edges() == graph.number_of_edges()
