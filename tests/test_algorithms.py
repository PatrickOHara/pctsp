"""Tests for exact algorithms for PCTSP"""

# import networkx as nx
# from pctsp.algorithms import pctsp_branch_and_cut
# from tspwplib import total_prize, total_cost_networkx


# def test_pctsp_on_suurballes_graph(
#     suurballes_undirected_graph, suurballe_source
# ):
#     """Test the branch and cut algorithm on a small, undirected sparse graph"""
#     quota = 6
#     edge_list = pctsp_branch_and_cut(
#         suurballes_undirected_graph, quota, suurballe_source
    )
    # assert (
    #     total_prize(
    #         nx.get_node_attributes(suurballes_undirected_graph, "prize"),
    #         optimal_tour,
    #     )
    #     >= quota
    # )
    # assert total_cost_networkx(suurballes_undirected_graph, optimal_tour) > 0


# def test_pctsp_on_tspwplib(tspwplib_graph_tool, root):
#     """Test the branch and cut algorithm on a small, undirected sparse graph"""
#     quota = 6
#     optimal_tour = pctsp_branch_and_cut(tspwplib_graph_tool, quota, root)
#     assert total_prize(tspwplib_graph_tool.vp.prize, optimal_tour) >= quota
#     assert total_cost_graph_tool(tspwplib_graph_tool, optimal_tour) > 0
