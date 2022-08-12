"""Take the cross product of parameters"""

import itertools
from pathlib import Path
from typing import List, Optional
from uuid import uuid4

from tspwplib import (
    Generation,
    GraphName,
    ProfitsProblem,
    build_path_to_oplib_instance,
)
from tspwplib.problem import PrizeCollectingTSP
from tspwplib.types import LondonaqGraphName, EdgeWeightType
from tspwplib.utils import build_path_to_londonaq_yaml

from ..constants import FOUR_HOURS, LP_GAP_IMPROVEMENT_THRESHOLD
from ..vial import (
    AlgorithmName,
    BranchingStrategy,
    DatasetName,
    ModelParams,
    Preprocessing,
    DataConfig,
    Vial,
    EXACT_ALGORITHMS,
    HEURISTIC_ALGORITHMS,
    RELAXATION_ALGORITHMS,
)


def product_of_tspwplib_data_config(
    oplib_root: Path,
    alpha_list: List[int],
    kappa_list: List[Optional[int]],
    generation_list: List[Generation],
    graph_name_list: List[GraphName],
    cost_function_list: List[EdgeWeightType],
) -> List[DataConfig]:
    """All combinations of the parameters"""
    data_config_list = []
    for alpha, kappa, generation, graph_name, cost_function in itertools.product(
        alpha_list,
        kappa_list,
        generation_list,
        graph_name_list,
        cost_function_list,
    ):
        problem_path = build_path_to_oplib_instance(oplib_root, generation, graph_name)
        problem = ProfitsProblem().load(problem_path)
        data_config_list.append(
            DataConfig(
                alpha=alpha,
                cost_function=cost_function,
                dataset=DatasetName.tspwplib,
                kappa=kappa if kappa else None,
                generation=generation,
                graph_name=graph_name,
                quota=problem.get_quota(alpha),
                root=problem.get_root_vertex(normalize=False),
                triangle=0,
            )
        )
    return data_config_list


def product_of_londonaq_data_config(
    londonaq_root: Path,
    quota_list: List[int],
    graph_name_list: List[LondonaqGraphName],
) -> List[DataConfig]:
    """Iterate over data settings for the londonaq dataset"""
    problems = {}
    for name in graph_name_list:
        problem_path = build_path_to_londonaq_yaml(londonaq_root, name)
        problems[name] = PrizeCollectingTSP.from_yaml(problem_path)

    data_config_list = []
    for quota, name in itertools.product(quota_list, graph_name_list):
        data_config_list.append(
            DataConfig(
                cost_function=problems[name].edge_weight_type,
                dataset=DatasetName.londonaq,
                graph_name=name,
                quota=quota,
                root=problems[name].get_root_vertex(),
            )
        )
    return data_config_list


def product_of_model_params(
    algorithm_list: List[AlgorithmName],
    collapse_paths: bool = False,
    path_depth_limit: int = 2,
    step_size: int = 1,
    time_limit: float = FOUR_HOURS,
) -> List[ModelParams]:
    """Get the cross product of all param lists"""
    model_params_list = []
    for algorithm_name in algorithm_list:
        is_exact = algorithm_name in EXACT_ALGORITHMS
        model_params = ModelParams(
            algorithm=algorithm_name,
            is_exact=is_exact,
            is_heuristic=algorithm_name in HEURISTIC_ALGORITHMS,
            is_relaxation=algorithm_name in RELAXATION_ALGORITHMS,
        )
        if is_exact:
            model_params.branching_max_depth = -1
            model_params.branching_strategy = BranchingStrategy.RELPSCOST
            model_params.cost_cover_disjoint_paths = False
            model_params.cost_cover_shortest_path = False
            model_params.sec_disjoint_tour = is_exact
            model_params.sec_lp_gap_improvement_threshold = LP_GAP_IMPROVEMENT_THRESHOLD
            model_params.sec_maxflow_mincut = is_exact
            model_params.sec_max_tailing_off_iterations = -1
            model_params.sec_sepafreq = 1
            model_params.time_limit = time_limit
        if AlgorithmName.bfs_extension_collapse in (
            model_params.algorithm,
            model_params.heuristic,
        ):
            model_params.collapse_paths = collapse_paths
            model_params.path_depth_limit = path_depth_limit
            model_params.step_size = step_size
        model_params_list.append(model_params)

    return model_params_list


def product_of_preprocessing(
    remove_disconnected_components_list: List[bool],
    remove_leaves_list: List[bool],
    remove_one_connected_components_list: List[bool],
) -> List[Preprocessing]:
    """All combinations of preprocessing settings"""
    preprocessing_list = []
    for (
        remove_disconnected_components,
        remove_leaves,
        remove_one_connected_components,
    ) in itertools.product(
        remove_disconnected_components_list,
        remove_leaves_list,
        remove_one_connected_components_list,
    ):
        preprocessing_list.append(
            Preprocessing(
                disjoint_path_cutoff=False,
                remove_leaves=remove_leaves,
                remove_disconnected_components=remove_disconnected_components,
                remove_one_connected_components=remove_one_connected_components,
                shortest_path_cutoff=False,
            )
        )
    return preprocessing_list


def product_of_vials(
    data_config_list: List[DataConfig],
    model_params_list: List[ModelParams],
    preprocessing_list: List[Preprocessing],
) -> List[Vial]:
    """Given lists of parameters, return a list of Vials representing all possible
    combinations of the params"""
    vial_list = []
    for data_config, model_params, preprocessing in itertools.product(
        data_config_list, model_params_list, preprocessing_list
    ):
        vial_list.append(
            Vial(
                data_config=data_config,
                model_params=model_params,
                preprocessing=preprocessing,
                uuid=uuid4(),
            )
        )
    return vial_list
