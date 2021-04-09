"""Fixtures for preprocessing"""

import graph_tool as gt
import pytest


@pytest.fixture(scope="function")
def disconnected_graph() -> gt.Graph:
    """Graph with 3 components and 5 leaf vertices"""
    graph = gt.Graph(directed=False)
    graph.add_edge_list([(0, 1), (1, 2), (0, 2), (3, 4), (5, 6), (2, 7)])
    return graph
