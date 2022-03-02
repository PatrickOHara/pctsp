"""Tests for preprocessing a networkx graph"""

import copy
import networkx as nx
import pytest

from tspwplib import sparsify_uid
from pctsp import (
    remove_leaves,
    remove_components_disconnected_from_vertex,
    # remove_one_connected_components,
)

def remove_one_connected_components(graph: nx.Graph, root_vertex) -> nx.Graph:
    """Remove vertices that are not in the same bi-connected component as the root vertex

    Args:
        graph: Undirected graph
        root_vertex: Root vertex

    Returns:
        Undirected graph where all vertices are in the same bi-connected component

    Notes:
        The graph is mutated. Make a copy if you wish to keep the original input graph
    """
    root_component = set()
    for component in nx.biconnected_components(graph):
        # if root_vertex in component and len(component) >= 3:
        print(component)
        if root_vertex in component:
            # NOTE the 'update' here is essential incase the root is part of multiple bi-connected
            # components, but removing the root disconnects the remaining graph
            root_component.update(component)
    assert len(root_component) >= 1
    removed_vertices = set(graph.nodes()) - root_component
    graph_copy = nx.Graph(
        copy.deepcopy(graph)
    )  # NOTE unfreeze graph and make deep copy
    graph_copy.remove_nodes_from(removed_vertices)
    return graph_copy

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
        ([(0, 1), (1, 2), (2, 3)], 0, 0),
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
    no_leaves = remove_leaves(sparse)
    assert root in no_leaves
    assert no_leaves.degree(root) >= 2
    processed = remove_one_connected_components(no_leaves, root)
    assert root in processed