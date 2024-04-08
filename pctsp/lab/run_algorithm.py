"""Functions for running algorithms using vials"""

import datetime
from logging import Logger
from pathlib import Path
from typing import Mapping, Optional, no_type_check
import networkx as nx
from pyscipopt import Model

from tspwplib import (
    DisjointPaths,
    EdgeFunctionName,
    EdgesNotAdjacentException,
    NotSimpleException,
    SimpleEdgeFunction,
    SimpleEdgeList,
    Vertex,
    VertexFunctionName,
    asymmetric_from_undirected,
    biggest_vertex_id_from_graph,
    edge_list_from_walk,
    is_pctsp_yes_instance,
    order_edge_list,
    remove_self_loops_from_edge_list,
    total_cost,
    total_prize,
    split_head,
    vertex_set_from_edge_list,
)

from ..algorithms import (
    extension_unitary_gain_collapse,
    extension_until_prize_feasible,
    solve_pctsp,
    find_cycle_from_bfs,
    path_extension_collapse,
    suurballes_heuristic,
    suurballes_tour_initialization,
)
from ..preprocessing import (
    remove_components_disconnected_from_vertex,
    remove_leaves,
    remove_one_connected_components,
    undirected_vertex_disjoint_paths_map,
    vertex_disjoint_cost_map,
)
from ..suurballe import suurballe_shortest_vertex_disjoint_paths
from ..vial import (
    AlgorithmName,
    Result,
    Vial,
)
from ..utils import get_pctsp_logger


def run_heuristic(
    graph: nx.Graph,
    quota: int,
    root_vertex: Vertex,
    algorithm_name: AlgorithmName,
    disjoint_paths_cost_map: SimpleEdgeFunction,
    disjoint_paths_map: Mapping[Vertex, DisjointPaths],
    logger: Logger = get_pctsp_logger("run_heuristic"),
    collapse_shortest_paths: Optional[bool] = None,
    path_depth_limit: Optional[int] = None,
    step_size: Optional[int] = None,
) -> SimpleEdgeList:
    """Run a Prize-collecting TSP heuristic"""
    # initialise the below algorithms with a simple cycle generated from a DFS search
    small_tour = []
    prize_dict = nx.get_node_attributes(graph, VertexFunctionName.prize.value)
    if algorithm_name in [
        AlgorithmName.suurballes_extension_collapse,
        AlgorithmName.suurballes_heuristic,
        AlgorithmName.suurballes_path_extension_collapse,
    ] and (not disjoint_paths_cost_map or not disjoint_paths_map):
        message = "To run Suurballe's heuristic you must pass a non-empty dictionary "
        message += "to disjoint_paths_cost_map and disjoint_paths_cost_map."
        raise ValueError(message)
    if algorithm_name in [
        AlgorithmName.bfs_extension_collapse,
        AlgorithmName.bfs_path_extension_collapse,
    ]:
        small_tour = find_cycle_from_bfs(graph, root_vertex)
    elif algorithm_name in [
        AlgorithmName.suurballes_extension_collapse,
        AlgorithmName.suurballes_path_extension_collapse,
    ]:
        small_tour = suurballes_tour_initialization(
            prize_dict, quota, disjoint_paths_cost_map, disjoint_paths_map
        )

    # run heuristic
    if algorithm_name == AlgorithmName.suurballes_heuristic:
        tour = suurballes_heuristic(
            prize_dict, quota, disjoint_paths_cost_map, disjoint_paths_map
        )
        edge_list = edge_list_from_walk(tour)
    elif algorithm_name in [
        AlgorithmName.bfs_extension_collapse,
        AlgorithmName.suurballes_extension_collapse,
    ]:
        # run extension with unitary gain then collapse
        extension_until_prize_feasible(graph, small_tour, quota)
        tour = extension_unitary_gain_collapse(
            graph,
            small_tour,
            quota,
            root_vertex,
        )
        edge_list = edge_list_from_walk(tour)

    elif algorithm_name in [
        AlgorithmName.bfs_path_extension_collapse,
        AlgorithmName.suurballes_path_extension_collapse,
    ]:
        # pylint: disable=simplifiable-if-expression
        collapse_paths: bool = True if collapse_shortest_paths else False
        depth_limit: int = (
            graph.number_of_nodes() if not path_depth_limit else path_depth_limit
        )
        step: int = 10 if not step_size else step_size
        tour = path_extension_collapse(
            graph,
            small_tour,
            root_vertex,
            quota,
            collapse_shortest_paths=collapse_paths,
            path_depth_limit=depth_limit,
            step_size=step,
        )
        edge_list = edge_list_from_walk(tour)
    else:
        raise NotImplementedError(
            f"Heuristic named {algorithm_name} is not implemented"
        )
    heuristic_is_yes_instance = is_pctsp_yes_instance(
        graph,
        quota,
        root_vertex,
        edge_list,
    )
    logger.info(
        "Heuristic %s obtained feasible solution: %s",
        algorithm_name.value,
        heuristic_is_yes_instance,
    )
    return edge_list


@no_type_check
def run_algorithm(
    graph: nx.Graph,
    vial: Vial,
    logger: Logger = get_pctsp_logger("run_algorithm"),
    vial_dir: Path = Path("."),
) -> Result:
    """Given parameters provided by the vial, run an algorithm on the preprocessed graph"""

    logger.info(
        "Running %s for vial %s",
        vial.model_params.algorithm.value,
        vial.uuid,
    )
    start_time = datetime.datetime.now(datetime.UTC)

    # run Suurballe's algorithm needed by a heuristic or by the cost cover inequalities
    biggest_vertex = biggest_vertex_id_from_graph(graph)
    asymmetric_graph = asymmetric_from_undirected(graph)
    tree = suurballe_shortest_vertex_disjoint_paths(
        asymmetric_graph,
        split_head(biggest_vertex, vial.data_config.root),
        weight="cost",
    )
    cost_map = vertex_disjoint_cost_map(tree, biggest_vertex)
    vertex_disjoint_paths_map = undirected_vertex_disjoint_paths_map(
        tree, biggest_vertex
    )

    if vial.model_params.is_heuristic:
        edge_list = run_heuristic(
            graph,
            vial.data_config.quota,
            vial.data_config.root,
            vial.model_params.algorithm,
            disjoint_paths_cost_map=cost_map,
            disjoint_paths_map=vertex_disjoint_paths_map,
            logger=logger,
            path_depth_limit=vial.model_params.path_depth_limit,
            step_size=vial.model_params.step_size,
        )

    elif vial.model_params.is_exact:
        heuristic_edge_list = []
        if vial.model_params.heuristic:
            heuristic_edge_list = run_heuristic(
                graph,
                vial.data_config.quota,
                vial.data_config.root,
                vial.model_params.heuristic,
                collapse_shortest_paths=vial.model_params.collapse_paths,
                disjoint_paths_cost_map=cost_map,
                disjoint_paths_map=vertex_disjoint_paths_map,
                logger=logger,
                path_depth_limit=vial.model_params.path_depth_limit,
                step_size=vial.model_params.step_size,
            )
            # if the heuristic is not a feasible solution, then ignore solution
            if not is_pctsp_yes_instance(
                graph,
                vial.data_config.quota,
                vial.data_config.root,
                heuristic_edge_list,
            ):
                heuristic_edge_list = []

        model = Model(problemName=str(vial.uuid), createscip=True, defaultPlugins=False)
        edge_list = solve_pctsp(
            model,
            graph,
            heuristic_edge_list,
            vial.data_config.quota,
            vial.data_config.root,
            branching_max_depth=vial.model_params.branching_max_depth,
            branching_strategy=vial.model_params.branching_strategy,
            cost_cover_disjoint_paths=vial.model_params.cost_cover_disjoint_paths,
            cost_cover_shortest_path=vial.model_params.cost_cover_shortest_path,
            disjoint_paths_cost=cost_map,
            logging_level=logger.level,
            name=str(vial.uuid),
            solver_dir=vial_dir,
            sec_disjoint_tour=vial.model_params.sec_disjoint_tour,
            sec_lp_gap_improvement_threshold=vial.model_params.sec_lp_gap_improvement_threshold,
            sec_maxflow_mincut=vial.model_params.sec_maxflow_mincut,
            sec_max_tailing_off_iterations=vial.model_params.sec_max_tailing_off_iterations,
            sec_sepafreq=vial.model_params.sec_sepafreq,
            time_limit=vial.model_params.time_limit,
        )
        logger.info("Status of model: %s", model.getStatus())
        model.freeProb()

    else:
        raise NotImplementedError(
            f"{vial.model_params.algorithm.value} is not implemented"
        )

    # end time
    end_time = datetime.datetime.now(datetime.UTC)

    try:
        edge_list = remove_self_loops_from_edge_list(edge_list)
        edge_list = order_edge_list(edge_list)
    except (EdgesNotAdjacentException, NotSimpleException) as exc:
        warning_message = "%s Ignore this warning if the solver stopped early."
        warning_message += "It means no feasible solution was found."
        logger.warning(warning_message, exc)

    # calculate the sum of edges in solution that are not self loops
    objective = total_cost(
        nx.get_edge_attributes(graph, EdgeFunctionName.cost.value), edge_list
    )
    vertex_set = vertex_set_from_edge_list(edge_list)
    prize = total_prize(
        nx.get_node_attributes(graph, VertexFunctionName.prize.value), vertex_set
    )
    logger.info(
        "Tour has %s prize, %s cost and %s edges",
        prize,
        objective,
        len(edge_list),
    )
    # get and return result object
    result = Result(
        vial_uuid=vial.uuid,
        objective=objective,
        prize=prize,
        edge_list=edge_list,
        start_time=start_time,
        end_time=end_time,
    )
    return result


def run_preprocessing(
    graph: nx.Graph, vial: Vial, logger: Logger = get_pctsp_logger("run_algorithm")
) -> nx.Graph:
    """Run any preprocessing algorithms to reduce the size of the graph"""

    if vial.preprocessing.remove_disconnected_components:
        num_vertices = graph.number_of_nodes()
        num_edges = graph.number_of_edges()
        graph = remove_components_disconnected_from_vertex(graph, vial.data_config.root)
        logger.info(
            "Removed %s disconnected vertices and %s edges ",
            num_vertices - graph.number_of_nodes(),
            num_edges - graph.number_of_edges(),
        )
    if vial.preprocessing.remove_leaves:
        num_vertices = graph.number_of_nodes()
        num_edges = graph.number_of_edges()
        graph = remove_leaves(graph)

        logger.info(
            "Removed %s leaf vertices and %s edges ",
            num_vertices - graph.number_of_nodes(),
            num_edges - graph.number_of_edges(),
        )
    if vial.preprocessing.remove_one_connected_components:
        num_vertices = graph.number_of_nodes()
        num_edges = graph.number_of_edges()
        graph = remove_one_connected_components(graph, vial.data_config.root)
        logger.info(
            "Removed %s vertices and %s edges not bi-connected to the root vertex.",
            num_vertices - graph.number_of_nodes(),
            num_edges - graph.number_of_edges(),
        )
    if vial.preprocessing.shortest_path_cutoff:
        raise NotImplementedError("shortest path cutoff not yet implemented")
    if vial.preprocessing.disjoint_path_cutoff:
        raise NotImplementedError("disjoint path cutoff not yet implemented")
    return graph
