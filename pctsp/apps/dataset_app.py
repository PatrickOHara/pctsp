"""Dataset app"""

from typing import Any, Dict, List
from pathlib import Path
import networkx as nx
import pandas as pd
import typer
from tspwplib import (
    BaseTSP,
    Generation,
    ProfitsProblem,
    build_path_to_londonaq_yaml,
    build_path_to_oplib_instance,
    metricness,
    mst_cost,
    rename_edge_attributes,
    rename_node_attributes,
    sparsify_uid,
    total_cost,
    total_prize,
)
from ..preprocessing import remove_one_connected_components
from ..vial import DatasetName

from .options import LondonaqRootOption, OPLibRootOption
from ..compare import params

dataset_app = typer.Typer(name="dataset", help="Making and summarizing datasets")


@dataset_app.command(name="metricness")
def metricness_of_dataset(
    dataset: DatasetName,
    londonaq_root: Path = LondonaqRootOption,
    oplib_root: Path = OPLibRootOption,
) -> pd.DataFrame:
    """Create a pandas dataframe of the metricness and write to CSV"""
    dataset_stats: Dict[str, List[Any]] = {
        "num_nodes": [],
        "num_edges": [],
        "total_cost": [],
        "total_prize": [],
        "metricness": [],
    }
    names = []
    if dataset == DatasetName.londonaq:
        names = params.LONDONAQ_GRAPH_NAME_LIST
    elif dataset == DatasetName.tspwplib:
        names = params.TSPLIB_GRAPH_NAME_LIST
    for graph_name in names:
        # load the graph
        if dataset == DatasetName.londonaq:
            problem_path = build_path_to_londonaq_yaml(londonaq_root, graph_name)
            tsp = BaseTSP.from_yaml(problem_path)
        elif dataset == DatasetName.tspwplib:
            problem_path = build_path_to_oplib_instance(
                oplib_root,
                Generation.gen3,
                graph_name,
            )
            # load the problem from file
            problem = ProfitsProblem().load(problem_path)
            tsp = BaseTSP.from_tsplib95(problem)
        # get the graph in networkx
        graph = tsp.get_graph()
        rename_edge_attributes(graph, {"weight": "cost"}, del_old_attr=True)
        try:  # londonaq dataset
            rename_node_attributes(graph, {"demand": "prize"}, del_old_attr=True)
        except KeyError:  # tsplib dataset
            nx.set_node_attributes(graph, problem.get_node_score(), name="prize")

        # if removing edges
        if dataset == DatasetName.tspwplib:
            graph = sparsify_uid(graph, 5)
            new_cost = mst_cost(graph, cost_attr="cost")
            nx.set_edge_attributes(graph, new_cost, name="cost")

        # preprocessing
        graph = remove_one_connected_components(graph, tsp.depots[0])

        # count the number of edges, vertices, total prize, total cost and the metricness
        dataset_stats["num_nodes"].append(graph.number_of_nodes())
        dataset_stats["num_edges"].append(graph.number_of_edges())
        dataset_stats["total_cost"].append(
            total_cost(nx.get_edge_attributes(graph, "cost"), list(graph.edges()))
        )
        dataset_stats["total_prize"].append(
            total_prize(nx.get_node_attributes(graph, "prize"), list(graph.nodes()))
        )
        dataset_stats["metricness"].append(metricness(graph))
    df = pd.DataFrame(dataset_stats, index=names)
    print(df)
    if dataset == DatasetName.londonaq:
        filepath = londonaq_root / "londonaq_dataset.csv"
    elif dataset == DatasetName.tspwplib:
        filepath = oplib_root / "tsplib_dataset.csv"
    df.index = df.index.rename("graph_name")
    df.to_csv(filepath, index=True)
    return df
