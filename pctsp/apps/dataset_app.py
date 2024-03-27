"""Dataset app"""

import itertools
from typing import Dict, List
from pathlib import Path
import networkx as nx
import pandas as pd
import typer
from tspwplib import (
    BaseTSP,
    EdgeWeightType,
    Generation,
    ProfitsProblem,
    asymmetric_from_undirected,
    biggest_vertex_id_from_graph,
    build_path_to_londonaq_yaml,
    build_path_to_oplib_instance,
    metricness,
    mst_cost,
    rename_edge_attributes,
    rename_node_attributes,
    sparsify_uid,
    split_head,
    total_cost,
    total_prize,
)
from ..compare import params
from ..preprocessing import (
    remove_one_connected_components,
    undirected_vertex_disjoint_paths_map,
    vertex_disjoint_cost_map,
)
from ..suurballe import suurballe_shortest_vertex_disjoint_paths
from ..utils import get_pctsp_logger
from ..vial import DatasetName

from .options import LondonaqRootOption, OPLibRootOption

dataset_app = typer.Typer(name="dataset", help="Making and summarizing datasets")


@dataset_app.command(name="stats")
def stats_of_dataset(
    dataset: DatasetName,
    londonaq_root: Path = LondonaqRootOption,
    oplib_root: Path = OPLibRootOption,
) -> pd.DataFrame:
    """Create a pandas dataframe of the metricness and write to CSV"""
    logger = get_pctsp_logger("dataset-stats")
    dataset_stats: List[Dict[str, float]] = []
    names = []
    index = None
    if dataset == DatasetName.londonaq:
        logger.info("Calculating stats for londonaq dataset.")
        for graph_name in params.LONDONAQ_GRAPH_NAME_LIST:
            logger.info("Loading %s", graph_name.value)
            problem_path = build_path_to_londonaq_yaml(londonaq_root, graph_name)
            tsp = BaseTSP.from_yaml(problem_path)
            graph = tsp.get_graph()
            rename_edge_attributes(graph, {"weight": "cost"}, del_old_attr=True)
            rename_node_attributes(graph, {"demand": "prize"}, del_old_attr=True)
            logger.info("Calculating stats for %s", graph_name.value)
            dataset_stats.append(get_graph_stats(graph, tsp.depots[0]))
            names.append(graph_name.value)
        index = pd.Index(names, name="graph_name")

    elif dataset == DatasetName.tspwplib:
        for graph_name, gen, cost, kappa in itertools.product(
            params.TSPLIB_GRAPH_NAME_LIST,
            Generation,
            params.TSPLIB_COST_FUNCTIONS,
            params.TSPLIB_KAPPA_LIST,
        ):
            logger.info(
                "Loading %s on generation %s with cost %s and kappa %s",
                graph_name.value,
                gen.value,
                cost.value,
                kappa,
            )
            problem_path = build_path_to_oplib_instance(
                oplib_root,
                gen,
                graph_name,
            )
            # load the problem from file
            problem = ProfitsProblem().load(problem_path)
            tsp = BaseTSP.from_tsplib95(problem)
            graph = tsp.get_graph()
            nx.set_node_attributes(graph, problem.get_node_score(), name="prize")
            rename_edge_attributes(graph, {"weight": "cost"}, del_old_attr=True)
            graph = sparsify_uid(graph, kappa)
            if cost == EdgeWeightType.MST:
                new_cost = mst_cost(graph, cost_attr="cost")
                nx.set_edge_attributes(graph, new_cost, name="cost")
            logger.info("Calculating stats for %s", graph_name.value)
            dataset_stats.append(get_graph_stats(graph, tsp.depots[0]))
            names.append((graph_name.value, gen.value, cost.value, kappa))
        index = pd.MultiIndex.from_tuples(
            names, names=["graph_name", "generation", "cost_function", "kappa"]
        )

    logger.info("Creating dataframe from dataset stats.")
    df = pd.DataFrame(dataset_stats, index=index)
    print(df)
    if dataset == DatasetName.londonaq:
        filepath = londonaq_root / "londonaq_dataset.csv"
    elif dataset == DatasetName.tspwplib:
        filepath = oplib_root / "tsplib_dataset.csv"
    logger.info("Writing dataframe to CSV at %s", filepath)
    df.to_csv(filepath, index=True)
    return df


def get_graph_stats(graph: nx.Graph, root_vertex: int) -> Dict[str, float]:
    """Calculate features such as the number of edges, vertices, total prize,
    total cost and the metricness.
    """
    instance_stats = {}
    instance_stats["num_nodes"] = graph.number_of_nodes()
    instance_stats["num_edges"] = graph.number_of_edges()
    instance_stats["total_cost"] = total_cost(
        nx.get_edge_attributes(graph, "cost"), list(graph.edges())
    )
    og_prize = total_prize(nx.get_node_attributes(graph, "prize"), list(graph.nodes()))
    instance_stats["total_prize"] = og_prize
    try:
        instance_stats["metricness"] = metricness(graph)
    except nx.exception.NetworkXException:  # NOTE change to NotConnectedException
        largest_component_graph = graph.subgraph(
            max(nx.connected_components(graph), key=len)
        )
        instance_stats["metricness"] = metricness(largest_component_graph)

    # evaluate the largest prize of any least-cost vertex-disjoint paths
    biggest_vertex = biggest_vertex_id_from_graph(graph)
    asymmetric_graph = asymmetric_from_undirected(graph)
    tree = suurballe_shortest_vertex_disjoint_paths(
        asymmetric_graph,
        split_head(biggest_vertex, root_vertex),
        weight="cost",
    )
    vertex_disjoint_paths_map = undirected_vertex_disjoint_paths_map(
        tree, biggest_vertex
    )
    biggest_prize = 0
    biggest_vertex = None
    prize_map = nx.get_node_attributes(graph, "prize")
    for u, (path1, path2) in vertex_disjoint_paths_map.items():
        prize = (
            total_prize(prize_map, path1)
            + total_prize(prize_map, path2)
            - prize_map[u]
            - prize_map[root_vertex]
        )
        if prize > biggest_prize:
            biggest_prize = prize
            biggest_vertex = u
    instance_stats["biggest_disjoint_prize"] = biggest_prize
    instance_stats["disjoint_prize_ratio"] = float(biggest_prize) / float(og_prize)
    instance_stats["max_disjoint_paths_cost"] = max(
        vertex_disjoint_cost_map(tree, biggest_vertex).values()
    )

    # preprocessing
    graph = remove_one_connected_components(graph, root_vertex)

    # re-evaluate stats after preprocessing
    instance_stats["preprocessed_num_nodes"] = graph.number_of_nodes()
    instance_stats["preprocessed_num_edges"] = graph.number_of_edges()
    instance_stats["preprocessed_total_cost"] = total_cost(
        nx.get_edge_attributes(graph, "cost"), list(graph.edges())
    )
    pp_prize = total_prize(nx.get_node_attributes(graph, "prize"), list(graph.nodes()))
    instance_stats["preprocessed_total_prize"] = pp_prize
    instance_stats["preprocessed_metricness"] = metricness(graph)
    instance_stats["preprocessed_prize_ratio"] = float(pp_prize) / float(og_prize)
    return instance_stats
