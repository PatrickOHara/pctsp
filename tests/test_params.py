"""Test experiment parameters"""

import networkx as nx
from tspwplib import (
    build_path_to_oplib_instance,
    ProfitsProblem,
    BaseTSP,
    metricness,
    GraphName,
)


def test_tsplib_graphs(oplib_root, generation):
    """Check the graphs have the desired properties"""
    for graph_name in [GraphName.att48, GraphName.st70, GraphName.eil76]:
        # the graph is metric
        filepath = build_path_to_oplib_instance(oplib_root, generation, graph_name)
        problem = ProfitsProblem.load(filepath)
        tsp = BaseTSP.from_tsplib95(problem)
        G = tsp.get_graph()
        nx.set_edge_attributes(G, tsp.edge_weights, name="cost")
        print(graph_name, tsp.edge_weight_type)
        assert metricness(G) == 1
