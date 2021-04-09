"""Tests for Suurballe's algorithm"""

import sys
import networkx as nx
from tspwplib.converter import asymmetric_from_undirected
from pctsp.algorithms import (
    adjust_edge_cost,
    adjust_edge_cost_for_graph,
    edge_disjoint_path_cost,
    extract_suurballe_edge_disjoint_paths,
    is_ancestor,
    find_parents_in_shortest_path_tree,
    preorder,
    postorder,
    suurballe_shortest_vertex_disjoint_paths,
    SuurballeTree,
)
from pctsp.constants import NULL_VERTEX


def test_adjust_edge_cost():
    """Test adjust edge cost function"""
    assert adjust_edge_cost(5, 5, 10) == 0
    assert adjust_edge_cost(5, 7, 10) == 2


def test_adjusted_edge_cost_for_graph(
    suurballes_directed_graph, suurballe_source, expected_adjusted_cost_function
):
    """Test the adjust cost on whole graph"""
    distance_from_source = nx.single_source_dijkstra_path_length(
        suurballes_directed_graph, suurballe_source
    )
    actual_adjusted_cost = adjust_edge_cost_for_graph(
        suurballes_directed_graph, distance_from_source
    )
    # compare actual adjusted cost against expected adjusted cost
    for u, item in expected_adjusted_cost_function.items():
        for v, expected_cost in item.items():
            assert expected_cost == actual_adjusted_cost[u][v]


def test_find_parents_in_shortest_path_tree(
    suurballes_directed_graph, suurballe_source
):
    """Test finding the parent of every vertex in the shortest path tree"""
    path_from_source = nx.single_source_dijkstra_path(
        suurballes_directed_graph, suurballe_source
    )
    parents_in_tree = find_parents_in_shortest_path_tree(path_from_source)
    assert parents_in_tree[suurballe_source] == NULL_VERTEX
    assert parents_in_tree[1] == suurballe_source
    assert parents_in_tree[2] == suurballe_source
    assert parents_in_tree[3] == 1
    assert parents_in_tree[4] == 1
    assert parents_in_tree[5] == 2
    assert parents_in_tree[6] == 4
    assert parents_in_tree[7] == 5


def test_preorder(tree):
    """Test the preorder sort"""
    actual_order = preorder(tree, 0)
    assert list(nx.dfs_preorder_nodes(tree, source=0)) == actual_order


def test_postorder(tree):
    """Test post order sort"""
    actual_order = postorder(tree, 0)
    assert list(nx.dfs_postorder_nodes(tree, source=0)) == actual_order


def test_is_ancestor(tree):
    """Test ancestors are correctly identified for descendants"""
    postorder(tree, 0)
    preorder(tree, 0)
    assert is_ancestor(tree, 0, 0)
    assert is_ancestor(tree, 0, 1)
    assert is_ancestor(tree, 0, 2)
    assert is_ancestor(tree, 0, 3)
    assert is_ancestor(tree, 0, 4)
    assert not is_ancestor(tree, 1, 0)


def test_suurballe_tree_init(suurballes_directed_graph, suurballe_source):
    """Test the init function of the Suurballe tree"""
    distance_from_source, path_from_source = nx.single_source_dijkstra(
        suurballes_directed_graph, suurballe_source
    )
    parents_in_tree = find_parents_in_shortest_path_tree(path_from_source)
    tree = SuurballeTree(suurballe_source, distance_from_source, parents_in_tree)
    assert len(tree.parent) == 0
    assert len(tree.children) == 0
    assert len(tree.edges_incident_to_vertex) == 0
    assert len(tree.heap_queue) == tree.number_of_nodes()
    assert nx.is_weakly_connected(tree)
    assert nx.is_directed_acyclic_graph(tree)


def test_suurballe(
    suurballes_directed_graph,
    suurballe_source,
    expected_children_after_suurballe,
    expected_tentative_distance,
    expected_edges_incident_to_vertex,
    expected_labeled,
    expected_tentative_predecessor,
    expected_process_cause,
) -> None:
    """Test Suurballe's algorithm on the input graph from the 1984 paper"""
    T = suurballe_shortest_vertex_disjoint_paths(
        suurballes_directed_graph, suurballe_source
    )
    assert T.children == expected_children_after_suurballe
    assert T.tentative_distance == expected_tentative_distance
    assert T.edges_incident_to_vertex == expected_edges_incident_to_vertex
    assert expected_labeled == T.labeled
    assert expected_tentative_predecessor == T.tentative_predecessor
    assert expected_process_cause == T.process_cause


def test_edge_disjoint_path_cost(suurballes_directed_graph, suurballe_source):
    """Test the cost of the disjoint paths is correct"""
    T = suurballe_shortest_vertex_disjoint_paths(
        suurballes_directed_graph, suurballe_source
    )
    for vertex in T:
        disjoint_cost = edge_disjoint_path_cost(T, vertex)
        if T.labeled[vertex]:
            assert (
                disjoint_cost
                == 2 * T.distance_from_source[vertex] + T.tentative_distance[vertex]
            )
        else:
            assert disjoint_cost == sys.maxsize


def test_extract_suurballe_edge_disjoint_paths(
    suurballes_directed_graph,
    suurballe_source,
    expected_disjoint_paths,
):
    """Test paths are extracted from the tree"""
    tree = suurballe_shortest_vertex_disjoint_paths(
        suurballes_directed_graph, suurballe_source
    )
    for u in tree:
        left_path, right_path = extract_suurballe_edge_disjoint_paths(
            tree, suurballe_source, u
        )
        assert left_path == expected_disjoint_paths[u][0]
        assert right_path == expected_disjoint_paths[u][1]


def test_suurballe_on_tspwplib(tspwplib_graph):
    """Test Suurballe's works on tspwplib instances"""
    asymmetric_graph = asymmetric_from_undirected(tspwplib_graph)
    suurballe_source = list(asymmetric_graph.nodes())[0]
    distance_from_source = nx.shortest_path_length(asymmetric_graph, suurballe_source)
    tree = suurballe_shortest_vertex_disjoint_paths(
        asymmetric_graph, suurballe_source, weight="cost"
    )
    for vertex in tree:
        assert 2 * distance_from_source[vertex] <= edge_disjoint_path_cost(tree, vertex)
