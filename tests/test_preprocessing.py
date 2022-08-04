"""Tests for preprocessing a networkx graph"""

import networkx as nx
import pytest

from tspwplib import sparsify_uid
from pctsp.preprocessing import (
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
        ([(0, 1), (1, 2), (2, 0), (0, 3), (3, 4), (0, 4), (4, 5)], 0, 5),
        ([(0, 1), (0, 2), (1, 2)], 0, 3),
        ([(0, 1), (0, 2), (1, 2), (0, 3), (3, 4), (4, 5), (3, 5), (5, 7)], 0, 3),
    ],
)
def test_remove_one_connected_components(edges, source_vertex, size_of_processed_graph):
    """Test vertices not in the same bi-connected component as the root as removed"""
    G = nx.Graph(edges)
    G = remove_one_connected_components(G, source_vertex)
    assert source_vertex in G
    assert G.number_of_nodes() == size_of_processed_graph


@pytest.mark.parametrize("k", [1, 2, 5, 10])
def test_root_still_in_graph(tspwplib_graph, root, k):
    """Test that after applying preprocessing the root vertex remains in the graph"""
    sparse = sparsify_uid(tspwplib_graph, k)
    assert root in sparse
    processed = remove_one_connected_components(sparse, root)
    assert root in processed
