"""Tests for exact algorithms for PCTSP"""

import networkx as nx
from pyscipopt import Model, SCIP_STAGE
from tspwplib import (
    asymmetric_from_undirected,
    biggest_vertex_id_from_graph,
    edge_list_from_walk,
    is_complete_with_self_loops,
    order_edge_list,
    reorder_edge_list_from_root,
    total_cost_networkx,
    is_pctsp_yes_instance,
    split_head,
    total_prize_of_tour,
    walk_from_edge_list,
)
from pctsp.algorithms import random_tour_complete_graph, solve_pctsp, SummaryStats
from pctsp.constants import PCTSP_SUMMARY_STATS_YAML
from pctsp.preprocessing import vertex_disjoint_cost_map
from pctsp.suurballe import suurballe_shortest_vertex_disjoint_paths


def test_pctsp_on_suurballes_graph(
    suurballes_undirected_graph, root, logger_dir, time_limit
):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 6
    name = "test_pctsp_on_suurballes_graph"
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    edge_list = solve_pctsp(
        model,
        suurballes_undirected_graph,
        [],
        quota,
        root,
        branching_strategy=2,
        name=name,
        solver_dir=logger_dir,
        time_limit=time_limit,
    )
    assert len(edge_list) > 0
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    optimal_tour = walk_from_edge_list(ordered_edges)
    assert is_pctsp_yes_instance(
        suurballes_undirected_graph, quota, root, ordered_edges
    )
    assert total_cost_networkx(suurballes_undirected_graph, optimal_tour) == 20
    assert model.getProbName() == name
    assert (
        model.getNVars()
        == suurballes_undirected_graph.number_of_edges()
        + suurballes_undirected_graph.number_of_nodes()
    )
    assert model.getStage() == SCIP_STAGE.SOLVED
    assert model.getStatus() == "optimal"


def test_pctsp_on_tspwplib(sparse_tspwplib_graph, root, logger_dir, time_limit):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 30
    tspwplib_graph = sparse_tspwplib_graph
    name = "test_pctsp_on_tspwplib"
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    edge_list = solve_pctsp(
        model,
        tspwplib_graph,
        [],
        quota,
        root,
        name=name,
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
    assert model.getStatus() in ("optimal", "timelimit")


def test_pctsp_with_heuristic(tspwplib_graph, root, logger_dir, time_limit):
    """Test adding an initial solution to solver"""
    quota = int(sum(nx.get_node_attributes(tspwplib_graph, "prize").values()) * 0.1)
    tour = random_tour_complete_graph(tspwplib_graph, root, quota)
    name = "test_pctsp_with_heuristic" + str(tspwplib_graph.graph["name"])
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    edge_list = edge_list_from_walk(tour)
    assert (
        total_prize_of_tour(nx.get_node_attributes(tspwplib_graph, "prize"), tour)
        >= quota
    )
    for (u, v) in edge_list:
        assert tspwplib_graph.has_edge(u, v)
    for i in range(len(edge_list) - 1):
        assert edge_list[i][1] == edge_list[i + 1][0]
    assert tspwplib_graph.graph["name"] in model.getProbName()
    assert logger_dir.exists()
    assert len(edge_list) >= 3
    assert is_pctsp_yes_instance(tspwplib_graph, quota, root, edge_list)
    assert is_complete_with_self_loops(tspwplib_graph)
    solve_pctsp(
        model,
        tspwplib_graph,
        edge_list,
        quota,
        root,
        name=name,
        solver_dir=logger_dir,
        time_limit=time_limit,
    )


def test_pctsp_cost_cover_shortest_path(
    tspwplib_graph,
    root,
    logger_dir,
    time_limit,
):
    """Test adding shortest path cost cover inequalities"""
    quota = 10  # small quota should promote more cost cover inequalities added
    name = "test_pctsp_cost_cover_shortest_path"
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    solve_pctsp(
        model,
        tspwplib_graph,
        [],
        quota,
        root,
        cost_cover_shortest_path=True,
        solver_dir=logger_dir,
        time_limit=time_limit,
    )


def run_cost_cover_disjoint_path(graph, quota, root, logger_dir, time_limit):
    """Run all the tests for the cost cover disjoint paths"""
    biggest_vertex = biggest_vertex_id_from_graph(graph)
    # convert to asymmetric graph and run Suurballe's
    asymmetric_graph = asymmetric_from_undirected(graph)
    tree = suurballe_shortest_vertex_disjoint_paths(
        asymmetric_graph,
        split_head(biggest_vertex, root),
        weight="cost",
    )
    # get mappings from vertex to cost (of vertex-disjoint path)
    cost_map = vertex_disjoint_cost_map(tree, biggest_vertex)
    name = "test_cost_cover"
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    optimal_tour = solve_pctsp(
        model,
        graph,
        [],
        quota,
        root,
        cost_cover_disjoint_paths=True,
        cost_cover_shortest_path=True,
        disjoint_paths_cost=cost_map,
        name=name,
        solver_dir=logger_dir,
        sec_disjoint_tour=True,
        sec_maxflow_mincut=True,
        time_limit=time_limit,
    )
    summary_path = logger_dir / PCTSP_SUMMARY_STATS_YAML
    assert summary_path.exists()
    summary = SummaryStats.from_yaml(summary_path)
    assert (
        summary.num_cost_cover_shortest_paths <= summary.num_cost_cover_disjoint_paths
    )
    return optimal_tour


def test_cost_cover_disjoint_paths_tspwplib(
    tspwplib_graph, root, logger_dir, time_limit
):
    """Test adding disjoint path cost cover inequalities"""
    quota = 10  # small quota should promote more cost cover inequalities added
    run_cost_cover_disjoint_path(tspwplib_graph, quota, root, logger_dir, time_limit)


def test_cost_cover_disjoint_paths_suurballes(
    suurballes_undirected_graph, root, logger_dir, time_limit
):
    """Test adding disjoint path  cost covers on suurballes graph"""
    quota = 1  # optimal solution is a triangle
    edge_list = run_cost_cover_disjoint_path(
        suurballes_undirected_graph,
        quota,
        root,
        logger_dir,
        time_limit,
    )
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    optimal_tour = walk_from_edge_list(ordered_edges)
    summary_path = logger_dir / PCTSP_SUMMARY_STATS_YAML
    summary = SummaryStats.from_yaml(summary_path)
    assert total_cost_networkx(suurballes_undirected_graph, optimal_tour) == 15
    assert summary.num_cost_cover_disjoint_paths == 5
    assert summary.num_cost_cover_shortest_paths == 1


def test_cycle_cover_tspwplib(tspwplib_graph, root, logger_dir, time_limit):
    """Test adding cycle cover inequalities on tspwplib graphs"""
    quota = 30  # small quota should promote more cost cover inequalities added
    name = "test_cycle_cover_tspwplib"
    model = Model(problemName=name, createscip=True, defaultPlugins=False)
    solve_pctsp(
        model,
        tspwplib_graph,
        [],
        quota,
        root,
        cycle_cover=True,
        solver_dir=logger_dir,
        time_limit=time_limit,
    )


def test_cycle_cover_grid8(grid8, root, logger_dir):
    """Test adding cycle cover inequalities on grid8 graph"""
    quota = 6  # expect exactly one cycle cover inquality to be added

    # ONLY cycle cover inequalities are added
    # turn off SEC - a feasible solution is still returned for this instance
    name = "test_cycle_cover_grid8"
    model = Model(problemName=name, createscip=True, defaultPlugins=False)

    edge_list = solve_pctsp(
        model,
        grid8,
        [],
        quota,
        root,
        cost_cover_disjoint_paths=False,
        cost_cover_shortest_path=False,
        cycle_cover=True,
        solver_dir=logger_dir,
        sec_disjoint_tour=False,
        sec_maxflow_mincut=False,
    )
    # load statistics - check number of cycle cover inequalities added
    summary_path = logger_dir / PCTSP_SUMMARY_STATS_YAML
    summary = SummaryStats.from_yaml(summary_path)
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    assert summary.num_sec_disjoint_tour == 0
    assert summary.num_sec_maxflow_mincut == 0
    assert summary.num_cycle_cover == 1
    assert is_pctsp_yes_instance(grid8, quota, root, ordered_edges)
    assert model.getStatus() == "optimal"
