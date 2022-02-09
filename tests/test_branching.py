from pctsp.libpypctsp import model_pctsp_bind
from pyscipopt import Model, SCIP_STAGE, SCIP_STATUS
import networkx as nx
import faulthandler
from tspwplib import is_pctsp_yes_instance, total_prize

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
    model: Model = model_pctsp_bind("scip", list(graph.edges()), prize_dict, cost_dict, quota, root, initial_yes_instance)
    print(model)
    assert model.getProbName() == "modelPrizeCollectingTSP"
    assert model.getNTotalNodes() == 1
    assert model.getNVars() == suurballes_undirected_graph.number_of_edges() + suurballes_undirected_graph.number_of_nodes()
    # model.optimize()
    assert model.getStage() == SCIP_STAGE.SOLVED
    assert model.getStatus() == "optimal"

def test_model_pctsp_tsplib(tspwplib_graph, root):
    quota = (int) 0.5 * total_prize(tspwplib_graph) 
