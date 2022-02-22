"""Fixtures for algorithms"""

from pathlib import Path
import sys
from typing import Dict, List, Set, Tuple
import networkx as nx
import pytest
import tspwplib.types as tp
from tspwplib import build_path_to_oplib_instance, ProfitsProblem, sparsify_uid
from pctsp import NULL_VERTEX

# pylint: disable=redefined-outer-name


# fixtures that use the tspwplib


@pytest.fixture(scope="function")
def tspwplib_graph(
    oplib_root,
    generation,
    graph_name,
) -> nx.Graph:
    """Test Suurballe's works on tspwplib instances"""
    filepath = build_path_to_oplib_instance(oplib_root, generation, graph_name)
    problem = ProfitsProblem.load(filepath)
    graph = problem.get_graph(normalize=True)
    return graph

@pytest.fixture(scope="function")
def sparse_tspwplib_graph(
    oplib_root,
    generation,
    graph_name,
) -> nx.Graph:
    """Test on a sparse instance of the TSPLIB dataset"""
    filepath = build_path_to_oplib_instance(oplib_root, generation, graph_name)
    problem = ProfitsProblem.load(filepath)
    graph = problem.get_graph(normalize=True)
    graph = sparsify_uid(graph, 5, seed=1)
    return graph

@pytest.fixture(scope="function")
def disconnected_graph() -> nx.Graph:
    """Graph with 3 components and 5 leaf vertices"""
    graph = nx.Graph()
    graph.add_edges_from([(0, 1), (1, 2), (0, 2), (3, 4), (5, 6), (2, 7)])
    return graph


# fixtures for suurballes algorithm


@pytest.fixture(scope="function")
def root() -> int:
    """Root vertex"""
    return 0


@pytest.fixture(scope="function")
def suurballes_vertices() -> List[int]:
    """Vertices"""
    return list(range(8))


@pytest.fixture(scope="function")
def suurballes_edges() -> Set[Tuple[int, int, int]]:
    """Weighted edges"""
    return [
        (0, 1, 3),
        (0, 4, 8),
        (0, 2, 2),
        (1, 3, 1),
        (1, 4, 4),
        (1, 5, 6),
        (2, 5, 5),
        (3, 6, 5),
        (4, 6, 1),
        (5, 7, 2),
        (7, 2, 3),
        (7, 6, 7),
    ]


def pytest_addoption(parser):
    """Options for filepaths for pytest-tspwplib"""
    group = parser.getgroup("pctsp")
    group.addoption(
        "--datasets-root",
        default="datasets",
        required=False,
        type=str,
        help="Filepath to test datasets directory",
    )


@pytest.fixture(scope="function")
def dataset_root(request) -> Path:
    """Root of test datasets"""
    return Path(request.config.getoption("--datasets-root"))


@pytest.fixture(scope="function")
def grid8(dataset_root) -> nx.Graph:
    """Undirected grid graph with 8 vertices"""
    filepath = dataset_root / "grid8.dot"
    G = nx.Graph(nx.drawing.nx_pydot.read_dot(filepath))
    G = nx.relabel.convert_node_labels_to_integers(G)
    for u, v, data in G.edges(data=True):
        G[u][v]["cost"] = int(data["cost"])
    nx.set_node_attributes(G, 1, name="prize")
    return G


@pytest.fixture(scope="function")
def suurballes_undirected_graph(suurballes_edges) -> nx.Graph:
    """Undirected suurballe's graph with prize of 1 on every vertex"""
    G = nx.Graph()
    G.add_weighted_edges_from(suurballes_edges, weight="cost")
    nx.set_node_attributes(G, 1, name="prize")
    return G


@pytest.fixture(scope="function")
def suurballes_directed_graph(suurballes_vertices, suurballes_edges) -> nx.DiGraph:
    """A digraph from Suurballe and Tarjan's 1984 paper"""
    G = nx.DiGraph()
    G.add_nodes_from(suurballes_vertices)
    G.add_weighted_edges_from(suurballes_edges)
    return G


@pytest.fixture(scope="function")
def expected_labeled(suurballes_vertices):
    """Expected labels at the end of Suurballe's algorithm"""
    return dict(
        zip(
            suurballes_vertices,
            [
                True,
                False,
                True,
                False,
                True,
                True,
                True,
                False,
            ],
        )
    )


def empty_list_lookup(vertices):
    """Vertex to empty list lookup"""
    return {v: [] for v in vertices}


@pytest.fixture(scope="function")
def expected_edges_incident_to_vertex(suurballes_vertices):
    """Expected value of the lookup i - empty lists"""
    return empty_list_lookup(suurballes_vertices)


@pytest.fixture(scope="function")
def expected_children_after_suurballe(suurballes_vertices):
    """Expected children of each vertex in the tree after Suurballe's algorithm"""
    # all empty apart from vertex C is a child of A
    children = empty_list_lookup(suurballes_vertices)
    children[1] = [3]
    return children


@pytest.fixture(scope="function")
def expected_adjusted_cost_function(suurballes_vertices) -> tp.EdgeFunction:
    """Expected value of the adjusted cost function of each edge"""
    return dict(
        zip(
            suurballes_vertices,
            [
                {1: 0, 2: 0, 4: 1},
                {3: 0, 5: 2, 4: 0},
                {5: 0},
                {6: 1},
                {6: 0},
                {7: 0},
                {},
                {2: 10, 6: 8},
            ],
        )
    )


@pytest.fixture(scope="function")
def expected_tentative_distance(suurballes_vertices) -> tp.VertexFunction:
    """Expected value of lookup d"""
    return dict(
        zip(
            suurballes_vertices,
            [
                0,
                sys.maxsize,
                12,
                sys.maxsize,
                1,
                2,
                2,
                sys.maxsize,
            ],
        )
    )


@pytest.fixture(scope="function")
def expected_tentative_predecessor(suurballes_vertices) -> tp.VertexLookup:
    """Expected values of p"""
    return dict(
        zip(
            suurballes_vertices,
            [
                NULL_VERTEX,
                NULL_VERTEX,
                7,
                NULL_VERTEX,
                0,
                1,
                3,
                NULL_VERTEX,
            ],
        )
    )


@pytest.fixture(scope="function")
def expected_process_cause(suurballes_vertices) -> tp.VertexLookup:
    """Expected value of q"""
    return dict(
        zip(
            suurballes_vertices,
            [
                NULL_VERTEX,
                NULL_VERTEX,
                5,
                NULL_VERTEX,
                0,
                0,
                4,
                NULL_VERTEX,
            ],
        )
    )


@pytest.fixture(scope="function")
def expected_disjoint_paths(suurballes_vertices) -> Dict[tp.Vertex, tp.DisjointPaths]:
    """The optimal disjoint paths for suurballes graph"""
    return dict(
        zip(
            suurballes_vertices,
            [
                ([0], []),
                ([], []),
                ([0, 1, 5, 7, 2], [0, 2]),
                ([], []),
                ([0, 4], [0, 1, 4]),
                ([0, 1, 5], [0, 2, 5]),
                ([0, 1, 3, 6], [0, 4, 6]),
                ([], []),
            ],
        )
    )


@pytest.fixture(scope="function")
def tree() -> nx.DiGraph:
    """A simple directed tree"""
    G = nx.DiGraph()
    G.add_edge(0, 1)
    G.add_edge(0, 2)
    G.add_edge(1, 3)
    G.add_edge(1, 4)
    G.add_edge(2, 6)
    G.add_edge(2, 7)
    assert nx.is_tree(G)
    return G


@pytest.fixture(scope="function")
def logger_dir(tmp_path_factory) -> Path:
    """Temp logging directory"""
    return tmp_path_factory.mktemp(".logger_dir")


@pytest.fixture(scope="function")
def stats_dir(tmp_path_factory) -> Path:
    """Temp stats directory"""
    return tmp_path_factory.mktemp(".stats")
