from pctsp.libpypctsp import basic_solve_pctsp_bind
from pyscipopt import Model, SCIP_STAGE, SCIP_STATUS
import networkx as nx
import faulthandler
from tspwplib import (
    is_pctsp_yes_instance,
    order_edge_list,
    reorder_edge_list_from_root,
    total_cost_networkx,
    total_prize,
    walk_from_edge_list,
)


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
    if initial_solution and is_pctsp_yes_instance(graph, quota, root, initial_solution):
        initial_yes_instance = initial_solution
    # print(type(model_ptr))
    # model_pctsp_bind(model_ptr, list(graph.edges()), prize_dict, cost_dict, quota, root, initial_yes_instance)
    name = "test_model_pctsp"
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    # model.freeTransform()
    sol_edges = basic_solve_pctsp_bind(
        model,
        list(graph.edges()),
        initial_yes_instance,
        cost_dict,
        prize_dict,
        quota,
        root,
        name,
    )
    print(model)
    assert model.getProbName() == name
    assert model.getNTotalNodes() == 1
    assert (
        model.getNVars()
        == suurballes_undirected_graph.number_of_edges()
        + suurballes_undirected_graph.number_of_nodes()
    )
    assert model.getStage() == SCIP_STAGE.SOLVED
    assert model.getStatus() == "optimal"
    sol_edges = order_edge_list(sol_edges)
    tour = walk_from_edge_list(sol_edges)
    sol = model.getBestSol()
    assert total_cost_networkx(suurballes_undirected_graph, tour) == model.getSolObjVal(
        sol
    )


def test_model_pctsp_tsplib(tspwplib_graph, root):
    graph = tspwplib_graph
    quota = 10
    cost_dict = nx.get_edge_attributes(graph, "cost")
    prize_dict = nx.get_node_attributes(graph, "prize")
    initial_solution = []
    name = "test_model_pctsp_tsplib"
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    sol_edges = basic_solve_pctsp_bind(
        model,
        list(graph.edges()),
        initial_solution,
        cost_dict,
        prize_dict,
        quota,
        root,
        "hello, world",
    )
    assert model.getNVars() == graph.number_of_edges()  # self loops already added
    assert model.getStage() == SCIP_STAGE.SOLVED
    assert model.getStatus() == "optimal"

    sol_edges = order_edge_list(sol_edges)
    tour = walk_from_edge_list(sol_edges)
    sol = model.getBestSol()
    assert total_cost_networkx(graph, tour) == model.getSolObjVal(sol)


if __name__ == "__main__":
    suurballes_edges = [
        (0, 1, 3),
        (0, 4, 8),
        (0, 2, 2),
        (1, 3, 1),
        (1, 4, 4),
        (1, 5, 6),
        (2, 5, 5),
        (3, 6, 5),
        (4, 6, 1),
        (5, 7, 2),
        (7, 2, 3),
        (7, 6, 7),
    ]
    G = nx.Graph()
    G.add_weighted_edges_from(suurballes_edges, weight="cost")
    nx.set_node_attributes(G, 1, name="prize")
    test_model_pctsp(G, 0)
