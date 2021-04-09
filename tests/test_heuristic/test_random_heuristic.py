"""Test heuristics that have some randomness"""

from tspwplib import total_prize
from pctsp.heuristic import random_tour_complete_graph


def test_random_tour_complete_graph(tspwplib_graph_tool):
    """Test random tours on complete graphs"""
    prize = total_prize(tspwplib_graph_tool.vp.prize, tspwplib_graph_tool.vertices())
    quota = int(float(prize) * 0.5)
    tour = random_tour_complete_graph(tspwplib_graph_tool, 0, quota)
    assert total_prize(tspwplib_graph_tool.vp.prize, tour) >= quota
