"""Experiment settings for comparing a relaxation against heuristics"""

from pathlib import Path
from typing import List

from tspwplib import (
    EdgeWeightType,
    Generation,
    GraphName,
)

from ..vial import (
    AlgorithmName,
    DatasetName,
    Vial,
)
from .product_of_params import (
    product_of_model_params,
    product_of_preprocessing,
    product_of_tspwplib_data_config,
    product_of_vials,
)


def disjoint_tours_vs_heuristics(
    dataset_name: DatasetName, dataset_root: Path
) -> List[Vial]:
    """Compare the extension heuristic against the Suurballe's heuristic"""
    model_params_list = product_of_model_params(
        [
            AlgorithmName.disjoint_tours_relaxation,
            AlgorithmName.bfs_extension_collapse,
            AlgorithmName.suurballes_heuristic,
        ]
    )
    if dataset_name == DatasetName.tspwplib:
        data_config_list = product_of_tspwplib_data_config(
            dataset_root,
            [25, 50, 75],
            [5, 10, 15, 20],
            [Generation.gen1, Generation.gen2, Generation.gen3],
            [GraphName.st70, GraphName.rat195, GraphName.a280],
            [EdgeWeightType.EUC_2D],
        )
    else:
        message = (
            f"{dataset_name} not a supported dataset for 'disjoint_tours_vs_heuristics'"
        )
        raise ValueError(message)
    preprocessing_list = product_of_preprocessing([False], [False], [True])
    return product_of_vials(data_config_list, model_params_list, preprocessing_list)
