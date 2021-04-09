"""Test removing vertices and edges"""

from graph_tool.generation import complete_graph
from pctsp.preprocessing import (
    remove_leaves,
    remove_components_disconnected_from_vertex,
)


def test_remove_leaves(disconnected_graph):
    """Test all leaves are removed"""
    # test for non complete graph
    graph = remove_leaves(disconnected_graph)
    assert graph.num_vertices() == 3
    assert graph.num_edges() == 3

    # test for complete graph
    complete = complete_graph(5)
    graph = remove_leaves(complete)
    assert complete.num_vertices() == graph.num_vertices()
    assert complete.num_edges() == graph.num_edges()


def test_remove_components(disconnected_graph):
    """Test disconnected components are removed from the graph"""
    graph = remove_components_disconnected_from_vertex(disconnected_graph, 0)
    assert graph.num_vertices() == 4
    assert graph.num_edges() == 4

    # test for complete graph
    complete = complete_graph(5)
    graph = remove_components_disconnected_from_vertex(complete, 0)
    assert complete.num_vertices() == graph.num_vertices()
    assert complete.num_edges() == graph.num_edges()
