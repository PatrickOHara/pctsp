from pctsp.libpypctsp import basic_solve_pctsp
from pyscipopt import Model, SCIP_STAGE, SCIP_STATUS
import networkx as nx
import faulthandler
from tspwplib import is_pctsp_yes_instance, order_edge_list, reorder_edge_list_from_root, total_cost_networkx, total_prize, walk_from_edge_list

def test_model_pctsp(suurballes_undirected_graph, root):
    quota = 6
    graph = suurballes_undirected_graph
    faulthandler.enable()
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
    name = "test_model_pctsp"
    model = Model(problemName=name, createscip=True, defaultPlugins=True)
    # model.freeTransform()
    sol_edges = basic_solve_pctsp(model, list(graph.edges()), initial_yes_instance, cost_dict, prize_dict, quota, root, name)
    print(model)
    assert model.getProbName() == name
    assert model.getNTotalNodes() == 1
    assert model.getNVars() == suurballes_undirected_graph.number_of_edges() + suurballes_undirected_graph.number_of_nodes()
    assert model.getStage() == SCIP_STAGE.SOLVED
    assert model.getStatus() == "optimal"
    sol_edges = order_edge_list(sol_edges)
    tour = walk_from_edge_list(sol_edges)
    sol = model.getBestSol()
    assert total_cost_networkx(suurballes_undirected_graph, tour) == model.getSolObjVal(sol)

# def test_model_pctsp_tsplib(tspwplib_graph, root):
#     graph = tspwplib_graph
#     quota = 10
#     cost_dict = nx.get_edge_attributes(graph, "cost")
#     prize_dict = nx.get_node_attributes(graph, "prize")
#     initial_solution = []
#     name = "modelPrizeCollectingTSP"
#     model = Model(problemName=name)
#     model.freeProb()
#     sol_edges = basic_solve_pctsp(model, list(graph.edges()), initial_solution, cost_dict, prize_dict, quota, root, "hello, world")
#     assert model.getNVars() == graph.number_of_edges()  # self loops already added
#     assert model.getStage() == SCIP_STAGE.SOLVED
#     assert model.getStatus() == "optimal"

#     sol_edges = order_edge_list(sol_edges)
#     tour = walk_from_edge_list(sol_edges)
#     sol = model.getBestSol()
#     assert total_cost_networkx(graph, tour) == model.getSolObjVal(sol)
