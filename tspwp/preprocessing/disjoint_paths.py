"""Preprocessing steps using vertex disjoint paths"""


from typing import Mapping
from tspwplib import (
    get_original_from_split_vertex,
    get_original_path_from_split_path,
    is_vertex_split_tail,
    DisjointPaths,
    Vertex,
    VertexFunction,
)
from ..algorithms import (
    edge_disjoint_path_cost,
    extract_suurballe_edge_disjoint_paths,
    SuurballeTree,
)


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
    vertex_disjoint_paths_map = dict()
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
    cost_map = dict()
    for vertex in tree:
        # get the cost and store in map
        if is_vertex_split_tail(biggest_vertex_id, vertex) and tree.labeled[vertex]:
            original_vertex = get_original_from_split_vertex(biggest_vertex_id, vertex)
            cost_map[original_vertex] = edge_disjoint_path_cost(tree, vertex)
    return cost_map
