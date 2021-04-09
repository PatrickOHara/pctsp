import networkx as nx
from pctsp.preprocessing import graph_from_edge_list


def test_graph_from_edge_list(suurballes_undirected_networkx_graph):
    cost_map = nx.get_edge_attributes(suurballes_undirected_networkx_graph, "cost")
    prize_map = nx.get_node_attributes(suurballes_undirected_networkx_graph, "prize")
    print(prize_map)
    assert graph_from_edge_list(
        list(suurballes_undirected_networkx_graph.edges()), prize_map, cost_map
    )
