"""Test heuristics"""

from tspwplib import total_prize, total_cost_graph_tool
from pctsp.heuristic import collapse, extend, extend_until_prize_feasible


def test_collapse(suurballes_undirected_graph_tool):
    """Test if a tour is collapsed"""
    tour = [0, 1, 4, 6, 7, 2, 0]
    quota = 5
    root_vertex = 0
    new_tour = collapse(suurballes_undirected_graph_tool, tour, quota, root_vertex)
    assert total_cost_graph_tool(suurballes_undirected_graph_tool, new_tour) == 16
    assert total_prize(suurballes_undirected_graph_tool.vp.prize, new_tour) >= quota
    assert new_tour[0] == new_tour[len(new_tour) - 1] == root_vertex


def test_extend(suurballes_undirected_graph_tool):
    """Test if a tour is extended"""
    tour = [0, 1, 3, 6, 7, 2, 0]
    for vertex in tour:
        assert suurballes_undirected_graph_tool.vertex(vertex)

    extended_tour = extend(suurballes_undirected_graph_tool, tour)
    assert 4 not in extended_tour
    assert 5 in extended_tour
    assert len(extended_tour) == len(tour) + 1


def test_extend_until_prize_feasible(suurballes_undirected_graph_tool):
    """Test is a tour is extended until its prize is above the threshold"""
    tour = [0, 1, 5, 2, 0]
    quota = 6
    extended_tour = extend_until_prize_feasible(
        suurballes_undirected_graph_tool, tour, quota
    )
    assert 7 in extended_tour
    assert (
        total_prize(suurballes_undirected_graph_tool.vp.prize, extended_tour) >= quota
    )
