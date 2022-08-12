"""Experiment settings for comparing heuristics"""

from pathlib import Path
from typing import List

from tspwplib import Generation

from ..vial import (
    AlgorithmName,
    DatasetName,
    ModelParams,
    Vial,
)
from . import params
from .product_of_params import (
    product_of_londonaq_data_config,
    product_of_preprocessing,
    product_of_tspwplib_data_config,
    product_of_vials,
)


def compare_heuristics(dataset_name: DatasetName, dataset_root: Path) -> List[Vial]:
    """Compare the Extension & Collapse heuristic against Suurballe's heuristic"""
    model_params_list = [
        ModelParams(  # Extension Collapse
            algorithm=AlgorithmName.bfs_extension_collapse,
            collapse_paths=False,
            is_exact=False,
            is_heuristic=True,
            is_relaxation=False,
            path_depth_limit=2,
            step_size=1,
        ),
        ModelParams(  # Path Extension Collapse
            algorithm=AlgorithmName.bfs_path_extension_collapse,
            collapse_paths=True,
            is_exact=False,
            is_heuristic=True,
            is_relaxation=False,
            path_depth_limit=None,  # NOTE no depth limit
            step_size=10,
        ),
        ModelParams(  # Suurballe's heuristic
            algorithm=AlgorithmName.suurballes_heuristic,
            is_exact=False,
            is_heuristic=True,
            is_relaxation=False,
        ),
    ]

    if dataset_name == DatasetName.tspwplib:
        data_config_list = product_of_tspwplib_data_config(
            dataset_root,
            params.TSPLIB_ALPHA_LIST,
            params.TSPLIB_KAPPA_LIST,
            list(Generation),
            params.TSPLIB_GRAPH_NAME_LIST,
            params.TSPLIB_COST_FUNCTIONS,
        )
    elif dataset_name == DatasetName.londonaq:
        data_config_list = product_of_londonaq_data_config(
            dataset_root,
            params.LONDONAQ_QUOTA_LIST,
            params.LONDONAQ_GRAPH_NAME_LIST,
        )
    else:
        raise ValueError(
            f"{dataset_name} is not a supported dataset for experiment 'compare_heuristics'"
        )
    preprocessing_list = product_of_preprocessing([False], [False], [True])
    return product_of_vials(data_config_list, model_params_list, preprocessing_list)
