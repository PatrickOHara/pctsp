"""Tests for exact algorithms for PCTSP"""

from tspwplib import (
    asymmetric_from_undirected,
    biggest_vertex_id_from_graph,
    edge_list_from_walk,
    order_edge_list,
    reorder_edge_list_from_root,
    total_cost_networkx,
    is_pctsp_yes_instance,
    split_head,
    walk_from_edge_list,
)
from pctsp import (
    pctsp_branch_and_cut,
    random_tour_complete_graph,
    suurballe_shortest_vertex_disjoint_paths,
    vertex_disjoint_cost_map,
    SummaryStats,
    PCTSP_SUMMARY_STATS_YAML,
)


def test_pctsp_on_suurballes_graph(
    suurballes_undirected_graph, root, logger_dir, metrics_filename, logger_filename
):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 6
    edge_list = pctsp_branch_and_cut(
        suurballes_undirected_graph,
        quota,
        root,
        log_scip_filename=logger_filename,
        metrics_filename=metrics_filename,
        output_dir=logger_dir,
    )
    assert len(edge_list) > 0
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    optimal_tour = walk_from_edge_list(ordered_edges)
    assert is_pctsp_yes_instance(
        suurballes_undirected_graph, quota, root, ordered_edges
    )
    assert total_cost_networkx(suurballes_undirected_graph, optimal_tour) == 20


def test_pctsp_on_tspwplib(
    tspwplib_graph, root, logger_dir, metrics_filename, logger_filename
):
    """Test the branch and cut algorithm on a small, undirected sparse graph"""
    quota = 30
    edge_list = pctsp_branch_and_cut(
        tspwplib_graph,
        quota,
        root,
        log_scip_filename=logger_filename,
        metrics_filename=metrics_filename,
        output_dir=logger_dir,
    )
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    assert len(edge_list) > 0
    optimal_tour = walk_from_edge_list(ordered_edges)
    assert is_pctsp_yes_instance(tspwplib_graph, quota, root, ordered_edges)
    assert total_cost_networkx(tspwplib_graph, optimal_tour) > 0


def test_pctsp_with_heuristic(
    tspwplib_graph, root, logger_dir, metrics_filename, logger_filename
):
    """Test adding an initial solution to solver"""
    quota = 30
    tour = random_tour_complete_graph(tspwplib_graph, root, quota)
    edge_list = edge_list_from_walk(tour)
    pctsp_branch_and_cut(
        tspwplib_graph,
        quota,
        root,
        initial_solution=edge_list,
        log_scip_filename=logger_filename,
        metrics_filename=metrics_filename,
        output_dir=logger_dir,
    )


def test_pctsp_cost_cover_shortest_path(
    tspwplib_graph, root, logger_dir, metrics_filename, logger_filename
):
    """Test adding shortest path cost cover inequalities"""
    quota = 10  # small quota should promote more cost cover inequalities added
    pctsp_branch_and_cut(
        tspwplib_graph,
        quota,
        root,
        cost_cover_shortest_path=True,
        log_scip_filename=logger_filename,
        metrics_filename=metrics_filename,
        output_dir=logger_dir,
    )


def run_cost_cover_disjoint_path(
    graph, quota, root, logger_dir, metrics_filename, logger_filename
):
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
    optimal_tour = pctsp_branch_and_cut(
        graph,
        quota,
        root,
        cost_cover_disjoint_paths=True,
        cost_cover_shortest_path=True,
        disjoint_paths_cost=cost_map,
        log_scip_filename=logger_filename,
        metrics_filename=metrics_filename,
        output_dir=logger_dir,
        sec_disjoint_tour=True,
        sec_maxflow_mincut=True,
    )
    summary_path = logger_dir / PCTSP_SUMMARY_STATS_YAML
    assert summary_path.exists()
    summary = SummaryStats.from_yaml(summary_path)
    assert (
        summary.num_cost_cover_shortest_paths <= summary.num_cost_cover_disjoint_paths
    )
    return optimal_tour


def test_cost_cover_disjoint_paths_tspwplib(
    tspwplib_graph, root, logger_dir, metrics_filename, logger_filename
):
    """Test adding disjoint path cost cover inequalities"""
    quota = 10  # small quota should promote more cost cover inequalities added
    run_cost_cover_disjoint_path(
        tspwplib_graph, quota, root, logger_dir, metrics_filename, logger_filename
    )


def test_cost_cover_disjoint_paths_suurballes(
    suurballes_undirected_graph, root, logger_dir, metrics_filename, logger_filename
):
    """Test adding disjoint path  cost covers on suurballes graph"""
    quota = 1  # optimal solution is a triangle
    edge_list = run_cost_cover_disjoint_path(
        suurballes_undirected_graph,
        quota,
        root,
        logger_dir,
        metrics_filename,
        logger_filename,
    )
    ordered_edges = reorder_edge_list_from_root(order_edge_list(edge_list), root)
    optimal_tour = walk_from_edge_list(ordered_edges)
    summary_path = logger_dir / PCTSP_SUMMARY_STATS_YAML
    summary = SummaryStats.from_yaml(summary_path)
    assert total_cost_networkx(suurballes_undirected_graph, optimal_tour) == 15
    assert summary.num_cost_cover_disjoint_paths == 5
    assert summary.num_cost_cover_shortest_paths == 1
