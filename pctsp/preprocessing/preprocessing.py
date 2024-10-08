"""Preprocessing of undirected input graphs"""

import copy
from typing import Mapping

import networkx as nx
from tspwplib import (
    get_original_from_split_vertex,
    get_original_path_from_split_path,
    is_vertex_split_tail,
    DisjointPaths,
    Vertex,
    VertexFunction,
)
from ..suurballe import (
    edge_disjoint_path_cost,
    extract_suurballe_edge_disjoint_paths,
    SuurballeTree,
)


def remove_components_disconnected_from_vertex(
    graph: nx.Graph, vertex: Vertex
) -> nx.Graph:
    """Get a new graph with only one connected component that contains the vertex

    Args:
        graph: Undirected graph to preprocess
        vertex: A vertex in the graph

    Returns:
        Graph containing all vertices connected to the given vertex
    """
    return graph.subgraph(nx.node_connected_component(graph, vertex))


def undirected_vertex_disjoint_paths_map(
    tree: SuurballeTree, biggest_vertex_id: int
) -> Mapping[Vertex, DisjointPaths]:
    """Given the output tree from suurballe's algorithm, get a mapping from a vertex
    in the undirected graph to the least-cost pair of disjoint paths from the root
    to the vertex in the undirected graph

    Args:
        tree: nx.DiGraph that is returned by Suurballe's algorithm
        biggest_vertex_id: The id of the biggest vertex in the undirected input graph

    Returns:
        Mapping from vertices to disjoint paths
    """
    vertex_disjoint_paths_map = {}
    for vertex in tree:
        if is_vertex_split_tail(biggest_vertex_id, vertex) and tree.labeled[vertex]:
            original_vertex = get_original_from_split_vertex(biggest_vertex_id, vertex)
            split_vertex_disjoint_path = extract_suurballe_edge_disjoint_paths(
                tree, tree.source, vertex
            )
            first_path = get_original_path_from_split_path(
                biggest_vertex_id, split_vertex_disjoint_path[0]
            )
            second_path = get_original_path_from_split_path(
                biggest_vertex_id, split_vertex_disjoint_path[1]
            )
            vertex_disjoint_paths_map[original_vertex] = (first_path, second_path)
    return vertex_disjoint_paths_map


def vertex_disjoint_cost_map(
    tree: SuurballeTree, biggest_vertex_id: int
) -> VertexFunction:
    """Mapping from a vertex to the cost of the vertex disjoint path from the source to the vertex

    Args:
        tree: nx.DiGraph that is returned by Suurballe's algorithm
        biggest_vertex_id: The id of the biggest vertex in the undirected input graph

    Returns:
        Mapping from vertices to cost of disjoint paths
    """
    cost_map = {}
    for vertex in tree:
        # get the cost and store in map
        if is_vertex_split_tail(biggest_vertex_id, vertex) and tree.labeled[vertex]:
            original_vertex = get_original_from_split_vertex(biggest_vertex_id, vertex)
            cost_map[original_vertex] = edge_disjoint_path_cost(tree, vertex)
    return cost_map


def degree_without_self_loops(graph: nx.Graph, vertex: Vertex) -> int:
    """Get the degree of a graph but not including any self loops

    Args:
        graph: Undirected graph
        vertex: Vertex

    Returns:
        Number of edges connected to the vertex not including itself
    """
    count = 0
    for u in graph.neighbors(vertex):
        if u != vertex:
            count += 1
    return count


def remove_leaves(graph: nx.Graph) -> nx.Graph:
    """Remove leaves and isolated self loops from the graph

    Args:
        graph: Undirected graph

    Returns:
        New graph with no leaves

    Notes:
        Self loops are not considered
    """
    leaf_vertices = set()
    leaf_vertex_found = False
    for vertex in graph:
        # remove self loops and isolated vertices (not including self loops)
        if degree_without_self_loops(graph, vertex) <= 1:
            leaf_vertices.add(vertex)
            leaf_vertex_found = True
    subgraph = graph.subgraph(set(graph.nodes()) - leaf_vertices)
    if leaf_vertex_found:
        return remove_leaves(subgraph)
    return subgraph


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
    root_component.add(root_vertex)
    for component in nx.biconnected_components(graph):
        if root_vertex in component and len(component) >= 3:
            # NOTE the 'update' here is essential incase the root is part of multiple bi-connected
            # components, but removing the root disconnects the remaining graph
            root_component.update(component)
    removed_vertices = set(graph.nodes()) - root_component
    graph_copy = nx.Graph(
        copy.deepcopy(graph)
    )  # NOTE unfreeze graph and make deep copy
    graph_copy.remove_nodes_from(removed_vertices)
    return graph_copy
