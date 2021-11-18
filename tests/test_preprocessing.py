"""Tests for preprocessing a networkx graph"""

import networkx as nx
import pytest

from pctsp import (
    remove_leaves,
    remove_components_disconnected_from_vertex,
    remove_one_connected_components,
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


@pytest.mark.parametrize(
    "edges,source_vertex,size_of_processed_graph",
    [
        ([(0, 1), (1, 2), (2, 3)], 0, 1),
        ([(0, 1), (0, 2), (1, 2)], 0, 3),
        ([(0, 1), (0, 2), (1, 2), (0, 3), (3, 4), (4, 5), (3, 5)], 0, 3),
    ],
)
def test_remove_one_connected_components(edges, source_vertex, size_of_processed_graph):
    """Test vertices not in the same bi-connected component as the root as removed"""
    G = nx.Graph(edges)
    G = remove_one_connected_components(G, source_vertex)
    assert G.number_of_nodes() == size_of_processed_graph
