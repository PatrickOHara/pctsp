"""Test experiment parameters"""

import networkx as nx
from tspwplib import (
    EdgeWeightType,
    build_path_to_oplib_instance,
    ProfitsProblem,
    BaseTSP,
    metricness,
)
from pctsp.compare import params


def test_tsplib_graphs(oplib_root, generation):
    """Check the graphs have the desired properties"""
    for graph_name in params.TSPLIB_GRAPH_NAME_LIST:
        # the graph is metric
        filepath = build_path_to_oplib_instance(oplib_root, generation, graph_name)
        problem = ProfitsProblem.load(filepath)
        tsp = BaseTSP.from_tsplib95(problem)
        G = tsp.get_graph()
        nx.set_edge_attributes(G, tsp.edge_weights, name="cost")
        print(graph_name, tsp.edge_weight_type)
        assert tsp.edge_weight_type == EdgeWeightType.EUC_2D
        assert metricness(G) == 1
