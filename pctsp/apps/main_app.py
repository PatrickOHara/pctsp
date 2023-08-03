"""Build the main app"""

from datetime import datetime
import math
from pathlib import Path
from typing import Dict, Optional, Tuple
from uuid import uuid4
import typer
from tspwplib import (
    EdgeWeightType,
    Generation,
    GraphName,
    LondonaqGraphName,
    ProfitsProblem,
    BaseTSP,
    build_path_to_londonaq_yaml,
    build_path_to_oplib_instance,
    rename_node_attributes,
    rename_edge_attributes,
)
from ..constants import (
    LP_GAP_IMPROVEMENT_THRESHOLD,
    SEC_MAX_TAILING_OFF_ITER,
    STRONG_BRANCHING_MAX_DEPTH,
)
from ..vial import (
    AlgorithmName,
    BranchingStrategy,
    DataConfig,
    DatasetName,
    ExperimentName,
    Experiment,
    ModelParams,
    Preprocessing,
    Vial,
    EXACT_ALGORITHMS,
    HEURISTIC_ALGORITHMS,
    RELAXATION_ALGORITHMS,
)
from ..compare import (
    cost_cover,
    dryrun,
    compare_heuristics,
    disjoint_tours_vs_heuristics,
    londonaq_alpha,
    simple_branch_cut,
    tailing_off,
)
from ..compare.exact_experiment import baseline
from ..lab import Lab, get_nbatches
from ..utils import get_pctsp_logger
from .options import (
    AlphaOption,
    CollapsePathsOption,
    GenerationOption,
    GraphNameOption,
    LabDirOption,
    LoggingLevelOption,
    LondonaqGraphNameOption,
    LondonaqRootOption,
    OPLibRootOption,
    PathDepthLimitOption,
    RemoveLeavesOption,
    StepSizeOption,
    TimeLimitOption,
)

from .csv_app import csv_app
from .dataset_app import dataset_app
from .plot_app import plot_app
from .tables_app import tables_app
from .utils_app import utils_app

app = typer.Typer(name="pctsp")
app.add_typer(csv_app)
app.add_typer(dataset_app)
app.add_typer(plot_app)
app.add_typer(tables_app)
app.add_typer(utils_app)


# pylint: disable=too-many-arguments,too-many-locals


tsplib_experiment_lookup = {
    ExperimentName.baseline: baseline,
    ExperimentName.compare_heuristics: compare_heuristics,
    ExperimentName.cost_cover: cost_cover,
    ExperimentName.disjoint_tours_vs_heuristics: disjoint_tours_vs_heuristics,
    ExperimentName.dryrun: dryrun,
    ExperimentName.simple_branch_cut: simple_branch_cut,
    ExperimentName.tailing_off: tailing_off,
}

londonaq_experiment_lookup = {
    ExperimentName.baseline: baseline,
    ExperimentName.compare_heuristics: compare_heuristics,
    ExperimentName.cost_cover: cost_cover,
    ExperimentName.dryrun: dryrun,
    ExperimentName.londonaq_alpha: londonaq_alpha,
    ExperimentName.tailing_off: tailing_off,
}


def select_root_and_lookup(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    oplib_root: Path,
    londonaq_root: Path,
) -> Tuple[Path, Dict]:
    """Select the root and lookup for the dataset"""
    lookup = tsplib_experiment_lookup
    root_dir = oplib_root
    if (
        dataset == DatasetName.tspwplib
        and not experiment_name in tsplib_experiment_lookup
    ):
        raise ValueError(f"{experiment_name} must be a TSPLIB95 experiment.")
    if dataset == DatasetName.londonaq:
        lookup = londonaq_experiment_lookup
        root_dir = londonaq_root
        if not experiment_name in londonaq_experiment_lookup:
            raise ValueError(f"{experiment_name} must be a londonaq experiment.")
    return root_dir, lookup


@app.command()
def lab(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    lab_dir: Path = LabDirOption,
    logging_level: int = LoggingLevelOption,
    londonaq_root: Path = LondonaqRootOption,
    oplib_root: Path = OPLibRootOption,
) -> None:
    """Run an experiment in a lab"""
    root_dir, lookup = select_root_and_lookup(
        dataset, experiment_name, oplib_root, londonaq_root
    )
    # setup experiment by adding vials
    logger = get_pctsp_logger(f"tspwp-lab-{experiment_name.value}", level=logging_level)
    experiment = Experiment(name=experiment_name, vials=[], timestamp=datetime.now())
    experiment.vials.extend(lookup[experiment_name](dataset, root_dir))

    # the lab has all the equipment needed to run the experiment
    pctsp_lab = Lab(
        lab_dir / dataset.value,
        logger=logger,
        londonaq_root=londonaq_root,
        oplib_root=oplib_root,
    )

    # write the experiment to file incase we get an error when running
    pctsp_lab.write_experiment_to_file(experiment)

    # run the experiment
    results = pctsp_lab.run_experiment(experiment)

    # write results to file to ensure we have a copy incase DB fails
    pctsp_lab.write_results_to_file(experiment.name, results)


@app.command()
def setup(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    lab_dir: Path = LabDirOption,
    logging_level: int = LoggingLevelOption,
    londonaq_root: Path = LondonaqRootOption,
    oplib_root: Path = OPLibRootOption,
) -> None:
    """Setup an experiment in a lab ready for batching"""
    root_dir, lookup = select_root_and_lookup(
        dataset, experiment_name, oplib_root, londonaq_root
    )
    # setup experiment by adding vials
    logger = get_pctsp_logger(f"tspwp-lab-{experiment_name.value}", level=logging_level)
    experiment = Experiment(name=experiment_name, vials=[], timestamp=datetime.now())
    experiment.vials.extend(lookup[experiment_name](dataset, root_dir))

    # the lab has all the equipment needed to run the experiment
    pctsp_lab = Lab(
        lab_dir / dataset.value,
        logger=logger,
        londonaq_root=londonaq_root,
        oplib_root=oplib_root,
    )

    # write the experiment to file incase we get an error when running
    pctsp_lab.write_experiment_to_file(experiment)


@app.command()
def batch(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    batch_start: int,
    batch_size: int,
    lab_dir: Path = LabDirOption,
    logging_level: int = LoggingLevelOption,
    londonaq_root: Path = LondonaqRootOption,
    only_missing_results: bool = False,
    oplib_root: Path = OPLibRootOption,
) -> None:
    """Batch part of an experiment"""
    logger = get_pctsp_logger(f"tspwp-lab-{experiment_name.value}", level=logging_level)
    # the lab has all the equipment needed to run the experiment
    pctsp_lab = Lab(
        lab_dir / dataset.value,
        logger=logger,
        londonaq_root=londonaq_root,
        oplib_root=oplib_root,
    )
    experiment = pctsp_lab.read_experiment_from_file(
        experiment_name, only_missing_results=only_missing_results
    )

    # run the experiment - only the batched vials are run
    pctsp_lab.run_batch(experiment, batch_start, batch_size)


@app.command()
def nbatches(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    batch_size: int,
    lab_dir: Path = LabDirOption,
    logging_level: int = LoggingLevelOption,
    londonaq_root: Path = LondonaqRootOption,
    oplib_root: Path = OPLibRootOption,
) -> None:
    """Get the number of batches for the experiment given a batch size"""
    if batch_size <= 0:
        raise ValueError("Batch size must be greater than zero")
    logger = get_pctsp_logger(f"pctsp-lab-{experiment_name.value}", level=logging_level)
    pctsp_lab = Lab(
        lab_dir / dataset.value,
        logger=logger,
        londonaq_root=londonaq_root,
        oplib_root=oplib_root,
    )
    experiment = pctsp_lab.read_experiment_from_file(experiment_name)
    num_batches = get_nbatches(len(experiment.vials), batch_size)
    print(num_batches)


@app.command()
def slurm(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    batch_size: int,
    lab_dir: Path = LabDirOption,
    logging_level: int = LoggingLevelOption,
    londonaq_root: Path = LondonaqRootOption,
    mem_per_cpu: int = typer.Option(12000, help="Memory per CPU (in MB)"),
    only_missing_results: bool = False,
    oplib_root: Path = OPLibRootOption,
    overwrite: bool = False,
    partition: str = typer.Option("cpu-batch", help="Name of slurm partition"),
    time_limit: float = TimeLimitOption,
) -> None:
    """Generate a Slurm script for batching"""
    logger = get_pctsp_logger(f"tspwp-lab-{experiment_name.value}", level=logging_level)
    pctsp_lab = Lab(
        lab_dir / dataset.value,
        logger=logger,
        londonaq_root=londonaq_root,
        oplib_root=oplib_root,
    )
    experiment = pctsp_lab.read_experiment_from_file(
        experiment_name, only_missing_results=only_missing_results
    )
    experiment_root = pctsp_lab.get_experiment_dir(experiment_name)
    num_batches = get_nbatches(len(experiment.vials), batch_size)
    slurm_filepath = experiment_root / f"{experiment_name.value}.slurm"
    output_filename = f"joboutput_{experiment_name.value}_%j.out"
    error_filename = f"joboutput_{experiment_name.value}_%j.err"

    two_days_in_hours = 2 * 24 * 3600
    expected_cpu_time = math.ceil(float(batch_size) * (time_limit / 3600.0))
    slurm_time_limit_hours = min(expected_cpu_time + 1, two_days_in_hours)

    if slurm_filepath.exists() and not overwrite:
        logger.error(
            "Slurm file already exists. Use --overwrite option to replace file."
        )
        raise FileExistsError(slurm_filepath)

    slurm_string = f"""#!/usr/bin/bash
#SBATCH --job-name=pctsp
#SBATCH --partition={partition}
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu={mem_per_cpu}
#SBATCH --time={slurm_time_limit_hours}:00:00
#SBATCH --output={output_filename}
#SBATCH --error={error_filename}
#SBATCH --array=0-{num_batches-1}
#SBATCH --exclude=emu-01

## Loop over each batch ##
start=$(($SLURM_ARRAY_TASK_ID * {batch_size}))
srun --ntasks=1 --nodes=1 pctsp batch {dataset.value} {experiment_name.value} $start {batch_size} \
    --lab-dir {str(lab_dir)} \
    --logging-level {logging_level} \
    --londonaq-root {londonaq_root} \
    --oplib-root {oplib_root} \
"""
    if only_missing_results:
        slurm_string += " --only-missing-results"
    slurm_string += "\n"
    slurm_filepath.write_text(slurm_string)
    logger.info("Slurm file written to %s", slurm_filepath)


@app.command(name="missing")
def missing(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    lab_dir: Path = LabDirOption,
) -> None:
    """Print vials that are missing results"""
    pctsp_lab = Lab(
        lab_dir / dataset.value,
    )
    experiment = pctsp_lab.read_experiment_from_file(experiment_name)
    missing_vials = pctsp_lab.missing_vials(experiment, experiment.vials)
    for v in missing_vials:
        if v.data_config.dataset == DatasetName.londonaq:
            print(v.uuid, v.data_config.graph_name, v.data_config.quota)
        elif v.data_config.dataset == DatasetName.tspwplib:
            print(
                v.uuid,
                v.data_config.graph_name,
                v.data_config.alpha,
                v.data_config.kappa,
            )
    missing_experiment = Experiment(
        name=experiment.name, vials=missing_vials, timestamp=experiment.timestamp
    )
    pctsp_lab.write_experiment_to_file(missing_experiment, only_missing_results=True)


@app.command(name="vial")
def vial(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    vial_uuid: str,
    lab_dir: Path = LabDirOption,
    logging_level: int = LoggingLevelOption,
    londonaq_root: Path = LondonaqRootOption,
    oplib_root: Path = OPLibRootOption,
) -> None:
    """Re-run a vial with the given UUID"""
    logger = get_pctsp_logger(f"pctsp-lab-{experiment_name.value}", level=logging_level)
    pctsp_lab = Lab(
        lab_dir / dataset.value,
        logger=logger,
        londonaq_root=londonaq_root,
        oplib_root=oplib_root,
    )
    experiment = pctsp_lab.read_experiment_from_file(experiment_name)
    vial_to_run = None
    for v in experiment.vials:
        if str(v.uuid) == vial_uuid:
            vial_to_run = v
    if not vial_to_run:
        raise ValueError(f"Vial with UUID {vial_uuid} not found.")
    result = pctsp_lab.run_experiment_from_vial_list(experiment, [vial_to_run])[0]
    print(result)


@app.command()
def nmissing(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    lab_dir: Path = LabDirOption,
) -> None:
    """Print vials that are missing results"""
    pctsp_lab = Lab(
        lab_dir / dataset.value,
    )
    experiment = pctsp_lab.read_experiment_from_file(experiment_name)
    missing_vials = pctsp_lab.missing_vials(experiment, experiment.vials)
    print(len(missing_vials), "out of", len(experiment.vials), "missing")


@app.command(name="infeasible")
def infeasible(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    lab_dir: Path = LabDirOption,
) -> None:
    """Print vials that are infeasible"""
    pctsp_lab = Lab(
        lab_dir / dataset.value,
    )
    experiment = pctsp_lab.read_experiment_from_file(experiment_name)
    infeasible_vials = pctsp_lab.infeasible_vials(experiment, experiment.vials)
    for v in infeasible_vials:
        print(v.uuid)


@app.command()
def londonaq(
    algorithm_name: AlgorithmName,
    graph_name: LondonaqGraphName = LondonaqGraphNameOption,
    cost_cover_disjoint_paths: bool = True,
    cost_cover_shortest_path: bool = False,
    lab_dir: Path = LabDirOption,
    logging_level: int = LoggingLevelOption,
    londonaq_root: Path = LondonaqRootOption,
    quota: int = 200,
    remove_leaves: bool = RemoveLeavesOption,
    time_limit: float = TimeLimitOption,
) -> None:
    """Run an algorithm for PCTSP on the londonaq dataset"""
    logger = get_pctsp_logger(
        f"tspwp-londonaq-{algorithm_name.value}", level=logging_level
    )

    problem_path = build_path_to_londonaq_yaml(londonaq_root, graph_name)
    tsp = BaseTSP.from_yaml(problem_path)

    # get the graph in networkx
    graph = tsp.get_graph()
    rename_edge_attributes(graph, {"weight": "cost"}, del_old_attr=True)
    rename_node_attributes(graph, {"demand": "prize"}, del_old_attr=True)

    # settings
    data_config = DataConfig(
        cost_function=tsp.edge_weight_type,
        dataset=DatasetName.londonaq,
        graph_name=graph_name,
        quota=quota,
        root=tsp.depots[0],
    )
    is_exact = algorithm_name in EXACT_ALGORITHMS
    model_params = ModelParams(
        algorithm=algorithm_name,
        is_exact=is_exact,
        is_heuristic=algorithm_name in HEURISTIC_ALGORITHMS,
        is_relaxation=algorithm_name in RELAXATION_ALGORITHMS,
    )
    if is_exact:
        model_params.branching_strategy = BranchingStrategy.STRONG_AT_TREE_TOP
        model_params.branching_max_depth = STRONG_BRANCHING_MAX_DEPTH
        model_params.cost_cover_disjoint_paths = cost_cover_disjoint_paths
        model_params.cost_cover_shortest_path = cost_cover_shortest_path
        model_params.sec_disjoint_tour = True
        model_params.sec_lp_gap_improvement_threshold = LP_GAP_IMPROVEMENT_THRESHOLD
        model_params.sec_maxflow_mincut = True
        model_params.sec_max_tailing_off_iterations = -1
        model_params.sec_sepafreq = 1
        model_params.time_limit = time_limit
    preprocessing = Preprocessing(
        disjoint_path_cutoff=False,
        remove_leaves=remove_leaves,
        remove_disconnected_components=False,
        remove_one_connected_components=True,
        shortest_path_cutoff=False,
    )
    londonaq_vial = Vial(
        data_config=data_config,
        model_params=model_params,
        preprocessing=preprocessing,
        uuid=uuid4(),
    )
    experiment = Experiment(
        name=ExperimentName.onerun, vials=[], timestamp=datetime.now()
    )
    experiment.vials.append(londonaq_vial)

    # the lab has all the equipment needed to run the experiment
    pctsp_lab = Lab(
        lab_dir / DatasetName.londonaq.value,
        logger=logger,
        londonaq_root=londonaq_root,
    )

    # write the experiment to file incase we get an error when running
    pctsp_lab.write_experiment_to_file(experiment)

    # run the experiment
    results = pctsp_lab.run_experiment(experiment)

    # write results to file to ensure we have a copy incase DB fails
    pctsp_lab.write_results_to_file(experiment.name, results)


@app.command()
def tsplib(
    algorithm_name: AlgorithmName,
    alpha: int = AlphaOption,
    branching_max_depth: int = STRONG_BRANCHING_MAX_DEPTH,
    collapse_paths: bool = CollapsePathsOption,
    cost_cover_disjoint_paths: bool = True,
    cost_cover_shortest_path: bool = False,
    cost_function: EdgeWeightType = EdgeWeightType.UNIFORM_RANDOM,
    generation: Generation = GenerationOption,
    graph_name: GraphName = GraphNameOption,
    heuristic: Optional[AlgorithmName] = None,
    kappa: Optional[int] = None,
    lab_dir: Path = LabDirOption,
    logging_level: int = LoggingLevelOption,
    oplib_root: Path = OPLibRootOption,
    path_depth_limit: int = PathDepthLimitOption,
    remove_leaves: bool = RemoveLeavesOption,
    sec_lp_gap_improvement_threshold: float = LP_GAP_IMPROVEMENT_THRESHOLD,
    sec_max_tailing_off_iterations: int = SEC_MAX_TAILING_OFF_ITER,
    step_size: int = StepSizeOption,
    time_limit: float = TimeLimitOption,
) -> None:
    """Run an algorithm for PCTSP on the tsplib95 dataset"""
    logger = get_pctsp_logger(
        f"tspwp-tsplib-{algorithm_name.value}", level=logging_level
    )

    problem_path = build_path_to_oplib_instance(oplib_root, generation, graph_name)
    problem = ProfitsProblem().load(problem_path)
    problem.edge_weight_type = cost_function

    # settings
    data_config = DataConfig(
        alpha=alpha,
        cost_function=cost_function,
        dataset=DatasetName.tspwplib,
        generation=generation,
        graph_name=graph_name,
        kappa=kappa,
        quota=problem.get_quota(alpha),
        root=problem.get_root_vertex(normalize=False),
        triangle=0,
    )
    is_exact = algorithm_name in EXACT_ALGORITHMS
    model_params = ModelParams(
        algorithm=algorithm_name,
        is_exact=is_exact,
        is_heuristic=algorithm_name in HEURISTIC_ALGORITHMS,
        is_relaxation=algorithm_name in RELAXATION_ALGORITHMS,
    )
    if is_exact:
        model_params.branching_max_depth = branching_max_depth
        model_params.branching_strategy = BranchingStrategy.STRONG_AT_TREE_TOP
        model_params.cost_cover_disjoint_paths = cost_cover_disjoint_paths
        model_params.cost_cover_shortest_path = cost_cover_shortest_path
        model_params.heuristic = heuristic
        model_params.sec_disjoint_tour = True
        model_params.sec_lp_gap_improvement_threshold = sec_lp_gap_improvement_threshold
        model_params.sec_maxflow_mincut = True
        model_params.sec_max_tailing_off_iterations = sec_max_tailing_off_iterations
        model_params.sec_sepafreq = 1
        model_params.time_limit = time_limit
    if AlgorithmName.bfs_extension_collapse in (
        model_params.algorithm,
        model_params.heuristic,
    ):
        model_params.collapse_paths = collapse_paths
        model_params.path_depth_limit = path_depth_limit
        model_params.step_size = step_size
    preprocessing = Preprocessing(
        disjoint_path_cutoff=False,
        remove_leaves=remove_leaves,
        remove_disconnected_components=False,
        remove_one_connected_components=False,
        shortest_path_cutoff=False,
    )
    tsplib_vial = Vial(
        data_config=data_config,
        model_params=model_params,
        preprocessing=preprocessing,
        uuid=uuid4(),
    )
    experiment = Experiment(
        name=ExperimentName.onerun, vials=[], timestamp=datetime.now()
    )
    experiment.vials.append(tsplib_vial)

    # the lab has all the equipment needed to run the experiment
    pctsp_lab = Lab(
        lab_dir / DatasetName.tspwplib.value,
        logger=logger,
        oplib_root=oplib_root,
    )

    # write the experiment to file incase we get an error when running
    pctsp_lab.write_experiment_to_file(experiment)

    # run the experiment
    results = pctsp_lab.run_experiment(experiment)

    # write results to file to ensure we have a copy incase DB fails
    pctsp_lab.write_results_to_file(experiment.name, results)
