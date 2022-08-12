"""A small 'dryrun' experiment that is useful for debugging and testing"""

from pathlib import Path
from typing import List
from tspwplib import (
    EdgeWeightType,
    Generation,
    GraphName,
    LondonaqGraphName,
)
from ..vial import (
    AlgorithmName,
    DatasetName,
    Vial,
)
from .product_of_params import (
    product_of_londonaq_data_config,
    product_of_model_params,
    product_of_preprocessing,
    product_of_tspwplib_data_config,
    product_of_vials,
)


def dryrun(dataset_name: DatasetName, dataset_root: Path) -> List[Vial]:
    """Run all algorithms on the smallest complete graph"""
    model_params_list = product_of_model_params(
        [
            AlgorithmName.bfs_extension_collapse,
            AlgorithmName.suurballes_heuristic,
            AlgorithmName.solve_pctsp,
        ],
        time_limit=10,  # 10 second time limit
    )
    if dataset_name == DatasetName.tspwplib:
        data_config_list = product_of_tspwplib_data_config(
            dataset_root,
            [50],
            [10],
            [Generation.gen1],
            [GraphName.st70],
            [EdgeWeightType.EUC_2D],
        )
    elif dataset_name == DatasetName.londonaq:
        data_config_list = product_of_londonaq_data_config(
            dataset_root,
            [200],
            [LondonaqGraphName.laqtinyA],
        )
    else:
        raise ValueError(
            f"{dataset_name} is not a supported dataset for experiment 'dryrun'"
        )
    preprocessing_list = product_of_preprocessing([True], [True], [True])
    return product_of_vials(data_config_list, model_params_list, preprocessing_list)
