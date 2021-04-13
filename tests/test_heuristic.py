"""Test heuristics for the prize collecting TSP"""

import networkx as nx
from tspwplib import (
    asymmetric_from_undirected,
    biggest_vertex_id_from_graph,
    edge_list_from_walk,
    split_head,
    total_cost,
    total_cost_networkx,
    total_prize,
)
from pctsp import (
    suurballe_shortest_vertex_disjoint_paths,
    collapse,
    extend,
    extend_until_prize_feasible,
    undirected_vertex_disjoint_paths_map,
    vertex_disjoint_cost_map,
    suurballes_heuristic,
)


def test_collapse(suurballes_undirected_graph, root):
    """Test if a tour is collapsed"""
    tour = [0, 1, 4, 6, 7, 2, 0]
    quota = 5
    new_tour = collapse(suurballes_undirected_graph, tour, quota, root)
    assert total_cost_networkx(suurballes_undirected_graph, new_tour) == 16
    assert (
        total_prize(
            nx.get_node_attributes(suurballes_undirected_graph, "prize"), new_tour
        )
        >= quota
    )
    assert new_tour[0] == new_tour[len(new_tour) - 1] == root


def test_extend(suurballes_undirected_graph):
    """Test if a tour is extended"""
    tour = [0, 1, 3, 6, 7, 2, 0]
    for vertex in tour:
        assert suurballes_undirected_graph.has_node(vertex)
    for edge in edge_list_from_walk(tour):
        assert suurballes_undirected_graph.has_edge(edge[0], edge[1])

    extended_tour = extend(suurballes_undirected_graph, tour)
    assert 4 not in extended_tour
    assert 5 in extended_tour
    assert len(extended_tour) == len(tour) + 1


def test_extend_until_prize_feasible(suurballes_undirected_graph):
    """Test is a tour is extended until its prize is above the threshold"""
    tour = [0, 1, 5, 2, 0]
    quota = 6
    extended_tour = extend_until_prize_feasible(
        suurballes_undirected_graph, tour, quota
    )
    assert 7 in extended_tour
    assert total_prize(suurballes_undirected_graph.vp.prize, extended_tour) >= quota


# def test_random_tour_complete_graph(tspwplib_graph_tool):
#     """Test random tours on complete graphs"""
#     prize = total_prize(tspwplib_graph_tool.vp.prize, tspwplib_graph_tool.vertices())
#     quota = int(float(prize) * 0.5)
#     tour = random_tour_complete_graph(tspwplib_graph_tool, 0, quota)
#     assert total_prize(tspwplib_graph_tool.vp.prize, tour) >= quota


def test_suurballes_heuristic(suurballes_undirected_graph, root):
    """Test Suurballe's heuristic"""
    # setup params
    quota = 5
    prize_map = dict(suurballes_undirected_graph.nodes(data="prize"))

    # convert to asymmetric graph and run Suurballe's
    biggest_vertex = biggest_vertex_id_from_graph(suurballes_undirected_graph)
    asymmetric_graph = asymmetric_from_undirected(suurballes_undirected_graph)
    assert (
        asymmetric_graph.number_of_edges()
        == 2 * suurballes_undirected_graph.number_of_edges()
        + suurballes_undirected_graph.number_of_nodes()
    )
    tree = suurballe_shortest_vertex_disjoint_paths(
        asymmetric_graph, split_head(biggest_vertex, root), weight="cost"
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
            nx.get_edge_attributes(suurballes_undirected_graph, "cost"),
            edge_list_from_walk(tour),
        )
        == 16
    )
    assert total_prize(prize_map, tour) >= quota
