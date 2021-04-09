"""Test Suurballe's heuristic"""

import networkx as nx
from tspwplib import (
    asymmetric_from_undirected,
    biggest_vertex_id_from_graph,
    edge_list_from_walk,
    split_head,
    total_cost,
    total_prize,
)
from pctsp.algorithms import suurballe_shortest_vertex_disjoint_paths
from pctsp.heuristic import suurballes_heuristic
from pctsp.preprocessing import (
    undirected_vertex_disjoint_paths_map,
    vertex_disjoint_cost_map,
)


def test_suurballes_heuristic(suurballes_undirected_networkx_graph, suurballe_source):
    """Test Suurballe's heuristic"""
    # setup params
    quota = 5
    prize_map = dict(suurballes_undirected_networkx_graph.nodes(data="prize"))

    # convert to asymmetric graph and run Suurballe's
    biggest_vertex = biggest_vertex_id_from_graph(suurballes_undirected_networkx_graph)
    asymmetric_graph = asymmetric_from_undirected(suurballes_undirected_networkx_graph)
    assert (
        asymmetric_graph.number_of_edges()
        == 2 * suurballes_undirected_networkx_graph.number_of_edges()
        + suurballes_undirected_networkx_graph.number_of_nodes()
    )
    tree = suurballe_shortest_vertex_disjoint_paths(
        asymmetric_graph, split_head(biggest_vertex, suurballe_source), weight="cost"
    )
    cost_map = vertex_disjoint_cost_map(tree, biggest_vertex)
    vertex_disjoint_paths_map = undirected_vertex_disjoint_paths_map(
        tree, biggest_vertex
    )

    # run the heuristic
    tour = suurballes_heuristic(prize_map, quota, cost_map, vertex_disjoint_paths_map)

    assert tour in ([0, 1, 5, 7, 2, 0], [0, 1, 5, 2, 0])
    assert (
        total_cost(
            nx.get_edge_attributes(suurballes_undirected_networkx_graph, "cost"),
            edge_list_from_walk(tour),
        )
        == 16
    )
    assert total_prize(prize_map, tour) >= quota
