"""Test heuristics for the prize collecting TSP"""

import networkx as nx
import pytest
from tspwplib import (
    asymmetric_from_undirected,
    biggest_vertex_id_from_graph,
    edge_list_from_walk,
    is_simple_cycle,
    split_head,
    total_cost,
    total_cost_networkx,
    total_prize,
    total_prize_of_tour,
    VertexFunctionName,
)
from pctsp.algorithms import (
    collapse,
    extension,
    extension_until_prize_feasible,
    find_cycle_from_bfs,
    path_extension_collapse,
    random_tour_complete_graph,
    suurballes_heuristic,
)
from pctsp.preprocessing import (
    undirected_vertex_disjoint_paths_map,
    vertex_disjoint_cost_map,
)
from pctsp.suurballe import suurballe_shortest_vertex_disjoint_paths


def test_collapse(suurballes_undirected_graph, root):
    """Test if a tour is collapsed"""
    tour = [0, 1, 4, 6, 7, 2, 0]
    quota = 5
    new_tour = collapse(suurballes_undirected_graph, tour, quota, root)
    assert total_cost_networkx(suurballes_undirected_graph, new_tour) == 16
    assert (
        total_prize_of_tour(
            nx.get_node_attributes(suurballes_undirected_graph, "prize"), new_tour
        )
        >= quota
    )
    assert new_tour[0] == new_tour[len(new_tour) - 1] == root


def test_extension(suurballes_undirected_graph, root):
    """Test if a tour is extended"""
    tour = [0, 1, 3, 6, 7, 2, 0]
    extended_tour = extension(suurballes_undirected_graph, tour, root)
    assert 4 not in extended_tour
    assert 5 in extended_tour
    assert len(extended_tour) == len(tour) + 1


def test_extension_tsplib(tspwplib_graph, root):
    """Test if a tour is extended on the tsplib dataset"""
    n = tspwplib_graph.number_of_nodes()
    tour = [0, 1, 2, n - 1, n - 2, 0]
    extended_tour = extension(tspwplib_graph, tour, root)
    prize_map = nx.get_node_attributes(tspwplib_graph, VertexFunctionName.prize.value)
    assert total_prize_of_tour(prize_map, extended_tour) > total_prize_of_tour(
        prize_map, tour
    )
    assert root in extended_tour
    assert is_simple_cycle(tspwplib_graph, extended_tour)
    for u in tspwplib_graph:
        assert extended_tour.count(u) < 2 or u == root


@pytest.mark.parametrize("step_size,path_depth_limit", [(1, 2), (1, 4)])
def test_extension_until_feasible(tspwplib_graph, root, step_size, path_depth_limit):
    """Test obtaining a feasible tour via extension"""
    quota = 20
    n = tspwplib_graph.number_of_nodes()
    tour = [0, 1, 2, n - 1, n - 2, 0]
    extended_tour = extension_until_prize_feasible(
        tspwplib_graph,
        tour,
        root,
        quota,
        step_size=step_size,
        path_depth_limit=path_depth_limit,
    )
    prize_map = nx.get_node_attributes(tspwplib_graph, VertexFunctionName.prize.value)
    assert total_prize_of_tour(prize_map, extended_tour) >= quota
    assert root in extended_tour
    assert is_simple_cycle(tspwplib_graph, extended_tour)
    for u in tspwplib_graph:
        assert extended_tour.count(u) < 2 or u == root


def test_path_extension_collapse(suurballes_undirected_graph, root):
    """Test if a tour is extended and collapsed using paths"""
    tour = [0, 1, 3, 6, 7, 2, 0]
    quota = 7
    new_tour = path_extension_collapse(
        suurballes_undirected_graph,
        tour,
        root,
        quota,
        collapse_shortest_paths=True,
        path_depth_limit=suurballes_undirected_graph.number_of_nodes(),
        step_size=10,
    )
    assert (
        total_prize_of_tour(
            nx.get_node_attributes(suurballes_undirected_graph, "prize"), new_tour
        )
        >= quota
    )


@pytest.mark.parametrize("step_size", [1, 2, 3])
def test_pec_tspwplib(tspwplib_graph, root, step_size):
    """Test obtaining a feasible tour via extension"""
    quota = 20
    n = tspwplib_graph.number_of_nodes()
    tour = [0, 1, 2, n - 1, n - 2, 0]
    extended_tour = path_extension_collapse(
        tspwplib_graph,
        tour,
        root,
        quota,
        collapse_shortest_paths=True,
        path_depth_limit=tspwplib_graph.number_of_nodes(),
        step_size=step_size,
    )
    prize_map = nx.get_node_attributes(tspwplib_graph, VertexFunctionName.prize.value)
    assert total_prize_of_tour(prize_map, extended_tour) >= quota
    assert root in extended_tour
    assert is_simple_cycle(tspwplib_graph, extended_tour)
    for u in tspwplib_graph:
        assert extended_tour.count(u) < 2 or u == root


def test_random_tour_complete_graph(tspwplib_graph, root):
    """Test random tours on complete graphs"""
    prize_dict = nx.get_node_attributes(tspwplib_graph, VertexFunctionName.prize.value)
    prize = total_prize(prize_dict, tspwplib_graph.nodes())
    quota = int(float(prize) * 0.5)
    tour = random_tour_complete_graph(tspwplib_graph, root, quota)
    assert total_prize_of_tour(prize_dict, tour) >= quota


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
    assert total_prize_of_tour(prize_map, tour) >= quota


def test_find_cycle_from_bfs(suurballes_undirected_graph, root):
    """Test finding a simple cycle including the root"""
    cycle = find_cycle_from_bfs(suurballes_undirected_graph, root)
    assert is_simple_cycle(suurballes_undirected_graph, cycle)
    assert root in cycle
    assert cycle[0] == root
    assert cycle[len(cycle) - 1] == root


def test_find_cycle_from_bfs_tsplib(tspwplib_graph, root):
    """Test finding simple cycle on tsplib graph"""
    cycle = find_cycle_from_bfs(tspwplib_graph, root)
    assert is_simple_cycle(tspwplib_graph, cycle)
    assert root in cycle
    assert cycle[0] == root
    assert cycle[len(cycle) - 1] == root
