"""Tests for different branching schemes"""

from pyscipopt import Model, SCIP_STAGE
from tspwplib import (
    order_edge_list,
    reorder_edge_list_from_root,
    sparsify_uid,
    total_cost_networkx,
    is_pctsp_yes_instance,
    walk_from_edge_list,
)
from pctsp import solve_pctsp


def test_strong_branching_at_tree_top(
    sparse_tspwplib_graph, root, logger_dir, time_limit
):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 30
    name = "test_strong_branching_at_tree_top"
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    tspwplib_graph = sparse_tspwplib_graph
    edge_list = solve_pctsp(
        model,
        tspwplib_graph,
        [],
        quota,
        root,
        branching_strategy=2,
        branching_max_depth=5,
        name=name,
        sec_disjoint_tour=True,
        sec_lp_gap_improvement_threshold=0.1,
        sec_maxflow_mincut=True,
        sec_max_tailing_off_iterations=-1,
        sec_sepafreq=1,
        solver_dir=logger_dir,
        time_limit=time_limit,
    )
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    assert len(edge_list) > 0
    optimal_tour = walk_from_edge_list(ordered_edges)
    assert is_pctsp_yes_instance(tspwplib_graph, quota, root, ordered_edges)
    assert total_cost_networkx(tspwplib_graph, optimal_tour) > 0
    assert model.getProbName() == name
    assert model.getNVars() == tspwplib_graph.number_of_edges()
    assert model.getStage() == SCIP_STAGE.SOLVED
    assert model.getStatus() == "optimal"


def test_avoid_tailing_off(sparse_tspwplib_graph, root, logger_dir):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 30
    name = "test_strong_branching_at_tree_top"
    tspwplib_graph = sparse_tspwplib_graph
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    edge_list = solve_pctsp(
        model,
        tspwplib_graph,
        [],
        quota,
        root,
        name=name,
        sec_disjoint_tour=True,
        sec_lp_gap_improvement_threshold=0.1,
        sec_maxflow_mincut=True,
        sec_max_tailing_off_iterations=2,
        sec_sepafreq=1,
        solver_dir=logger_dir,
    )
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    assert len(edge_list) > 0
    optimal_tour = walk_from_edge_list(ordered_edges)
    assert is_pctsp_yes_instance(tspwplib_graph, quota, root, ordered_edges)
    assert total_cost_networkx(tspwplib_graph, optimal_tour) > 0
    assert model.getStage() == SCIP_STAGE.SOLVED
    assert model.getStatus() == "optimal"


if __name__ == "__main__":
    from tspwplib import (
        build_path_to_oplib_instance,
        ProfitsProblem,
        Generation,
        GraphName,
    )
    from pathlib import Path
    import os

    oplib_root = Path(os.getenv("OPLIB_ROOT"))
    filepath = build_path_to_oplib_instance(
        oplib_root, Generation.gen2, GraphName.eil76
    )
    problem = ProfitsProblem.load(filepath)
    graph = problem.get_graph(normalize=True)
    graph = sparsify_uid(graph, 5, seed=1)
    # test_avoid_tailing_off(graph, 0, oplib_root)
    test_strong_branching_at_tree_top(graph, 0, ".logs", 20.0)
