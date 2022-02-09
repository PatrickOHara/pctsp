from pctsp.libpypctsp import model_pctsp_bind
from pyscipopt import Model
import networkx as nx
import faulthandler
from tspwplib import is_pctsp_yes_instance
import sys

def test_model_pctsp(suurballes_undirected_graph, root):
    quota = 6
    faulthandler.enable()
    assert faulthandler.is_enabled()
    graph = suurballes_undirected_graph
    # model = Model()
    # model_ptr = model.to_ptr(False)
    cost_dict = nx.get_edge_attributes(graph, "cost")
    prize_dict = nx.get_node_attributes(graph, "prize")
    initial_solution = []
    initial_yes_instance = []
    if initial_solution and is_pctsp_yes_instance(
        graph, quota, root, initial_solution
    ):
        initial_yes_instance = initial_solution
    # print(type(model_ptr))
    # model_pctsp_bind(model_ptr, list(graph.edges()), prize_dict, cost_dict, quota, root, initial_yes_instance)
    model = model_pctsp_bind("scip", list(graph.edges()), prize_dict, cost_dict, quota, root, initial_yes_instance)
    model.createProbBasic("test_model_pctsp")
    assert model.getProbName() == "test_model_pctsp"

