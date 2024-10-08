"""Find exact solutions to the Prize-collecting TSP"""

import itertools
from pathlib import Path
from typing import List

from tspwplib import Generation, GraphName

from ..constants import (
    FOUR_HOURS,
    LP_GAP_IMPROVEMENT_THRESHOLD,
    SEC_MAX_TAILING_OFF_ITER,
    STRONG_BRANCHING_MAX_DEPTH,
)
from ..vial import (
    AlgorithmName,
    BranchingStrategy,
    DatasetName,
    ModelParams,
    Vial,
)
from . import params
from .product_of_params import (
    product_of_londonaq_data_config,
    product_of_londonaq_data_config_from_alpha,
    product_of_model_params,
    product_of_preprocessing,
    product_of_tspwplib_data_config,
    product_of_vials,
)


def simple_branch_cut(dataset_name: DatasetName, dataset_root: Path) -> List[Vial]:
    """A simple branch and cut algorithm with only subtour elimination constraints added"""
    model_params_list = product_of_model_params(
        [
            AlgorithmName.bfs_extension_collapse,
            AlgorithmName.suurballes_heuristic,
            AlgorithmName.solve_pctsp,
        ],
        time_limit=FOUR_HOURS,
    )
    if dataset_name == DatasetName.tspwplib:
        data_config_list = product_of_tspwplib_data_config(
            dataset_root,
            [25, 50, 75],
            [None, 10, 20, 30],
            [Generation.gen1, Generation.gen2, Generation.gen3],
            [GraphName.st70, GraphName.rat195, GraphName.pr152, GraphName.gr229],
            params.TSPLIB_COST_FUNCTIONS,
        )
    else:
        raise ValueError(
            f"{dataset_name} is not a supported dataset for experiment 'simple_branch_cut'"
        )
    preprocessing_list = product_of_preprocessing([False], [False], [True])
    return product_of_vials(data_config_list, model_params_list, preprocessing_list)


def tailing_off(dataset_name, dataset_root: Path) -> List[Vial]:
    """Compare different methods for preventing SECs tailing off"""
    time_limit = FOUR_HOURS
    depth_limit = None  # no depth limit
    step = 10
    if dataset_name == DatasetName.tspwplib:
        data_config_list = product_of_tspwplib_data_config(
            dataset_root,
            [25, 75],
            [10, 20],
            list(Generation),
            params.TSPLIB_GRAPH_NAME_LIST,
            params.TSPLIB_COST_FUNCTIONS,
        )
        collapse_paths = True
        heuristic = AlgorithmName.suurballes_path_extension_collapse

    elif dataset_name == DatasetName.londonaq:
        data_config_list = product_of_londonaq_data_config(
            dataset_root,
            params.LONDONAQ_QUOTA_LIST,
            params.LONDONAQ_GRAPH_NAME_LIST,
        )
        heuristic = AlgorithmName.suurballes_path_extension_collapse
        collapse_paths = True
    else:
        raise ValueError(
            f"{dataset_name} is not a supported dataset for experiment 'cost_cover'"
        )

    # parameters for model params branching strategy
    strong_depth = [STRONG_BRANCHING_MAX_DEPTH]
    strategy = [BranchingStrategy.STRONG_AT_TREE_TOP, BranchingStrategy.DEFAULT]
    tailing_off_iter = [-1, SEC_MAX_TAILING_OFF_ITER, 10]
    gap_threshold = [LP_GAP_IMPROVEMENT_THRESHOLD]
    model_params_list = []
    for depth, strat, tail_iter, gap in itertools.product(
        strong_depth, strategy, tailing_off_iter, gap_threshold
    ):
        model_params_list.append(
            ModelParams(
                algorithm=AlgorithmName.solve_pctsp,
                branching_max_depth=depth,
                branching_strategy=strat,
                collapse_paths=collapse_paths,
                cost_cover_disjoint_paths=True,
                cost_cover_shortest_path=False,
                heuristic=heuristic,
                is_exact=True,
                is_heuristic=False,
                is_relaxation=False,
                path_depth_limit=depth_limit,
                sec_disjoint_tour=True,
                sec_lp_gap_improvement_threshold=gap,
                sec_maxflow_mincut=True,
                sec_max_tailing_off_iterations=tail_iter,
                sec_sepafreq=1,
                step_size=step,
                time_limit=time_limit,
            )
        )

    preprocessing_list = product_of_preprocessing([False], [False], [True])
    return product_of_vials(data_config_list, model_params_list, preprocessing_list)


def __get_cc_params() -> List[ModelParams]:
    time_limit = FOUR_HOURS
    depth_limit = None
    step = 10
    collapse_paths = True
    heuristic = AlgorithmName.suurballes_path_extension_collapse
    branching_strategy = BranchingStrategy.STRONG_AT_TREE_TOP

    no_cc_params = ModelParams(
        algorithm=AlgorithmName.solve_pctsp,
        branching_max_depth=STRONG_BRANCHING_MAX_DEPTH,
        branching_strategy=branching_strategy,
        collapse_paths=collapse_paths,
        cost_cover_disjoint_paths=False,
        cost_cover_shortest_path=False,
        heuristic=heuristic,
        is_exact=True,
        is_heuristic=False,
        is_relaxation=False,
        path_depth_limit=depth_limit,
        sec_disjoint_tour=True,
        sec_lp_gap_improvement_threshold=LP_GAP_IMPROVEMENT_THRESHOLD,
        sec_maxflow_mincut=True,
        sec_max_tailing_off_iterations=SEC_MAX_TAILING_OFF_ITER,
        sec_sepafreq=1,
        step_size=step,
        time_limit=time_limit,
    )

    shortest_path_cc_params = ModelParams(
        algorithm=AlgorithmName.solve_pctsp,
        branching_max_depth=STRONG_BRANCHING_MAX_DEPTH,
        branching_strategy=branching_strategy,
        collapse_paths=collapse_paths,
        cost_cover_disjoint_paths=False,
        cost_cover_shortest_path=True,
        heuristic=heuristic,
        is_exact=True,
        is_heuristic=False,
        is_relaxation=False,
        path_depth_limit=depth_limit,
        sec_disjoint_tour=True,
        sec_lp_gap_improvement_threshold=LP_GAP_IMPROVEMENT_THRESHOLD,
        sec_maxflow_mincut=True,
        sec_max_tailing_off_iterations=SEC_MAX_TAILING_OFF_ITER,
        sec_sepafreq=1,
        step_size=step,
        time_limit=time_limit,
    )
    disjoint_paths_cc_params = ModelParams(
        algorithm=AlgorithmName.solve_pctsp,
        branching_max_depth=STRONG_BRANCHING_MAX_DEPTH,
        branching_strategy=branching_strategy,
        collapse_paths=collapse_paths,
        cost_cover_disjoint_paths=True,
        cost_cover_shortest_path=False,
        heuristic=heuristic,
        is_exact=True,
        is_heuristic=False,
        is_relaxation=False,
        path_depth_limit=depth_limit,
        sec_disjoint_tour=True,
        sec_lp_gap_improvement_threshold=LP_GAP_IMPROVEMENT_THRESHOLD,
        sec_maxflow_mincut=True,
        sec_max_tailing_off_iterations=SEC_MAX_TAILING_OFF_ITER,
        sec_sepafreq=1,
        step_size=step,
        time_limit=time_limit,
    )
    return [
        no_cc_params,
        disjoint_paths_cc_params,
        shortest_path_cc_params,
    ]


def cost_cover(dataset_name: DatasetName, dataset_root: Path) -> List[Vial]:
    """Compare the branch and cut algorithm with and without cost cover inequalities"""
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
            f"{dataset_name} is not a supported dataset for experiment 'cost_cover'"
        )

    preprocessing_list = product_of_preprocessing([False], [False], [True])
    return product_of_vials(data_config_list, __get_cc_params(), preprocessing_list)


def cc_londonaq_alpha(dataset_name: DatasetName, dataset_root: Path) -> List[Vial]:
    """Run each cost cover algorithm for different values of alpha on londonaq"""
    if dataset_name != DatasetName.londonaq:
        raise ValueError(
            f"{dataset_name} is not a supported dataset for experiment 'cc_londonaq_alpha'"
        )
    data_config_list = product_of_londonaq_data_config_from_alpha(
        dataset_root,
        params.TSPLIB_ALPHA_LIST,
        params.LONDONAQ_GRAPH_NAME_LIST,
    )
    preprocessing_list = product_of_preprocessing([False], [False], [True])
    return product_of_vials(data_config_list, __get_cc_params(), preprocessing_list)


def baseline(dataset_name: DatasetName, dataset_root: Path) -> List[Vial]:
    """Baseline B&C algorithm with Extension and Collapse heuristic"""
    time_limit = FOUR_HOURS
    depth_limit = None
    step = 1
    collapse_paths = False
    heuristic = AlgorithmName.bfs_extension_collapse

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
            f"{dataset_name} is not a supported dataset for experiment 'cost_cover'"
        )
    branching_strategy = BranchingStrategy.STRONG_AT_TREE_TOP

    baseline_params = ModelParams(
        algorithm=AlgorithmName.solve_pctsp,
        branching_max_depth=STRONG_BRANCHING_MAX_DEPTH,
        branching_strategy=branching_strategy,
        collapse_paths=collapse_paths,
        cost_cover_disjoint_paths=False,
        cost_cover_shortest_path=False,
        heuristic=heuristic,
        is_exact=True,
        is_heuristic=False,
        is_relaxation=False,
        path_depth_limit=depth_limit,
        sec_disjoint_tour=True,
        sec_lp_gap_improvement_threshold=LP_GAP_IMPROVEMENT_THRESHOLD,
        sec_maxflow_mincut=True,
        sec_max_tailing_off_iterations=SEC_MAX_TAILING_OFF_ITER,
        sec_sepafreq=1,
        step_size=step,
        time_limit=time_limit,
    )

    model_params_list = [baseline_params]
    preprocessing_list = product_of_preprocessing([False], [False], [True])
    return product_of_vials(data_config_list, model_params_list, preprocessing_list)
