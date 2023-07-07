"""A lab runs experiments"""

import json
from logging import Logger
import math
from pathlib import Path
import traceback
from typing import List, Optional
from uuid import UUID
import networkx as nx


from tspwplib import (
    BaseTSP,
    EdgeWeightType,
    ProfitsProblem,
    build_path_to_londonaq_yaml,
    build_path_to_oplib_instance,
    mst_cost,
    rename_edge_attributes,
    rename_node_attributes,
    semi_mst_cost,
    sparsify_uid,
    uniform_random_cost,
)
from ..vial import (
    DatasetName,
    Experiment,
    ExperimentName,
    Result,
    DataConfig,
    Vial,
)
from ..utils import get_pctsp_logger
from .run_algorithm import run_algorithm, run_preprocessing

EXPERIMENT_FILENAME = "experiment.json"
MISSING_EXPERIMENT_PREFIX = "missing_"
RESULT_FILENAME = "results.json"


class Lab:
    """A lab runs experiments and contains all the "tools" needed to do so"""

    def __init__(
        self,
        lab_dir: Path,
        logger: Logger = get_pctsp_logger("Lab"),
        londonaq_root: Optional[Path] = None,
        oplib_root: Optional[Path] = None,
    ):
        self.logger = logger
        lab_dir.mkdir(exist_ok=True, parents=False)
        self.lab_dir = lab_dir
        self.londonaq_root = londonaq_root
        self.oplib_root = oplib_root

    def missing_vials(
        self, experiment: Experiment, vial_list: List[Vial]
    ) -> List[Vial]:
        """Return a sublist of vials that are missing results"""
        missing = []
        for vial in vial_list:
            try:
                Result.read_from_json_file(
                    self.get_vial_dir(experiment.name, vial.uuid), vial.uuid
                )
            except (FileNotFoundError, json.JSONDecodeError):
                missing.append(vial)
        return missing

    def infeasible_vials(
        self, experiment: Experiment, vial_list: List[Vial]
    ) -> List[Vial]:
        """Return a sublist of the vials that are infeasible"""
        infeasible = []
        for vial in vial_list:
            try:
                result = Result.read_from_json_file(
                    self.get_vial_dir(experiment.name, vial.uuid), vial.uuid
                )
                if result.prize < vial.data_config.quota:
                    infeasible.append(vial)
            except FileNotFoundError:
                pass
        return infeasible

    def run_batch(
        self,
        experiment: Experiment,
        batch_start: int,
        batch_size: int,
        only_missing_results: bool = False,
    ) -> List[Result]:
        """Run a batch of experiments in the lab"""
        # get the list of vials, beginning at batch start upto but not including batch end
        batch_end = min(batch_start + batch_size, len(experiment.vials))
        vial_list = experiment.vials
        if only_missing_results:
            vial_list = self.missing_vials(experiment, vial_list)
        vial_list = vial_list[batch_start:batch_end]
        return self.run_experiment_from_vial_list(experiment, vial_list)

    def run_experiment(self, experiment: Experiment) -> List[Result]:
        """Run an experiment in the lab"""
        return self.run_experiment_from_vial_list(experiment, experiment.vials)

    def run_experiment_from_vial_list(
        self, experiment: Experiment, vial_list: List[Vial]
    ) -> List[Result]:
        """Run a list of Vials"""
        result_list: List[Result] = []

        for vial in vial_list:
            # load the problem from file
            data_config: DataConfig = vial.data_config
            if data_config.dataset == DatasetName.tspwplib:
                if (
                    not self.oplib_root
                    or not self.oplib_root.exists()
                    or not self.oplib_root.is_dir()
                ):
                    raise FileNotFoundError(
                        f"Could not find OPLib directory: {self.oplib_root}"
                    )
                problem_path = build_path_to_oplib_instance(
                    self.oplib_root,
                    data_config.generation,
                    data_config.graph_name,
                )
                # load the problem from file
                problem = ProfitsProblem().load(problem_path)
                tsp = BaseTSP.from_tsplib95(problem)
            elif data_config.dataset == DatasetName.londonaq:
                if not self.londonaq_root or not self.londonaq_root.exists():
                    raise FileNotFoundError(
                        f"Could not find londonaq directory: {self.londonaq_root}"
                    )
                problem_path = build_path_to_londonaq_yaml(
                    self.londonaq_root, data_config.graph_name
                )
                tsp = BaseTSP.from_yaml(problem_path)

            # get the graph in networkx
            graph = tsp.get_graph()
            rename_edge_attributes(graph, {"weight": "cost"}, del_old_attr=True)
            try:  # londonaq dataset
                rename_node_attributes(graph, {"demand": "prize"}, del_old_attr=True)
            except KeyError:  # tsplib dataset
                nx.set_node_attributes(graph, problem.get_node_score(), name="prize")

            # if removing edges
            if data_config.kappa:
                graph = sparsify_uid(graph, data_config.kappa)

            # set a different cost function
            if data_config.cost_function == EdgeWeightType.UNIFORM_RANDOM:
                new_cost = uniform_random_cost(list(graph.edges()))
                nx.set_edge_attributes(graph, new_cost, name="cost")
            elif data_config.cost_function == EdgeWeightType.MST:
                new_cost = mst_cost(graph, cost_attr="cost")
                nx.set_edge_attributes(graph, new_cost, name="cost")
            elif data_config.cost_function == EdgeWeightType.SEMI_MST:
                new_cost = semi_mst_cost(graph, cost_attr="cost")
                nx.set_edge_attributes(graph, new_cost, name="cost")

            # logging
            vial_dir = self.get_vial_dir(experiment.name, vial.uuid)
            self.logger.info(
                "%s graph with %s vertices and %s edges with quota %s",
                vial.data_config.graph_name.value,
                graph.number_of_nodes(),
                graph.number_of_edges(),
                vial.data_config.quota,
            )

            try:
                # preprocessing
                self.logger.debug(
                    "%s vertices before preprocessing", graph.number_of_nodes()
                )
                graph = run_preprocessing(graph, vial, logger=self.logger)
                self.logger.debug(
                    "%s vertices after preprocessing", graph.number_of_nodes()
                )

                # run the algorithm and append the result
                result = run_algorithm(
                    graph,
                    vial,
                    logger=self.logger,
                    vial_dir=vial_dir,
                )
                result_list.append(result)
                result.write_to_json_file(self.get_vial_dir(experiment.name, vial.uuid))
                self.logger.info("Vial %s is done <^_^>", str(vial.uuid))

            # pylint: disable=broad-except
            # catch any exceptions then carry on to the next algorithm
            except Exception as oops:
                self.logger.exception(
                    "Vial %s on %s running with %s errored: ",
                    str(vial.uuid),
                    vial.data_config.graph_name,
                    vial.model_params.algorithm,
                )
                traceback.print_tb(oops.__traceback__)

        self.logger.info(
            "All vials have finished at timestamp %s",
            experiment.timestamp,
        )
        return result_list

    def get_experiment_dir(self, experiment_name: ExperimentName) -> Path:
        """Get the experiment directory path and create it if it does not exist"""
        experiment_dir: Path = self.lab_dir / experiment_name.value
        experiment_dir.mkdir(exist_ok=True, parents=False)
        return experiment_dir

    def get_vial_dir(self, experiment_name: ExperimentName, vial_id: UUID) -> Path:
        """Get the directory of a vial inside an experiment"""
        vial_dir = self.get_experiment_dir(experiment_name) / str(vial_id)
        vial_dir.mkdir(exist_ok=True, parents=False)
        return vial_dir

    def read_experiment_from_file(self, experiment_name: ExperimentName, only_missing_results: bool = False) -> Experiment:
        """Read experiment from local json file"""
        if only_missing_results:
            filepath = self.get_experiment_dir(experiment_name) / (MISSING_EXPERIMENT_PREFIX + EXPERIMENT_FILENAME)
        else:
            filepath = self.get_experiment_dir(experiment_name) / EXPERIMENT_FILENAME
        if not filepath.exists():
            raise FileNotFoundError(f"Could not find experiment json file: {filepath}")
        self.logger.info("Reading experiment JSON file from %s", filepath)
        with open(filepath, "r", encoding="utf-8") as json_file:
            experiment_dict = json.load(json_file)
            return Experiment(**experiment_dict)

    def write_experiment_to_file(self, experiment: Experiment, only_missing_results: bool = False) -> None:
        """Write experiment to local file"""
        if only_missing_results:
            filepath = self.get_experiment_dir(experiment.name) / (MISSING_EXPERIMENT_PREFIX + EXPERIMENT_FILENAME)
        else:
            filepath = self.get_experiment_dir(experiment.name) / EXPERIMENT_FILENAME
        self.logger.info("Writing experiment to %s", filepath)
        with open(filepath, "w", encoding="utf-8") as json_file:
            json.dump(json.loads(experiment.json()), json_file, indent=4)

    def write_results_to_file(
        self, experiment_name: ExperimentName, results: List[Result]
    ) -> None:
        """Write a backup of the results incase writing results to mongo fails"""
        filepath = self.get_experiment_dir(experiment_name) / RESULT_FILENAME
        result_list = [json.loads(result_model.json()) for result_model in results]
        with open(filepath, "w", encoding="utf-8") as json_file:
            json.dump(result_list, json_file)

    def read_results_from_file(self, experiment: Experiment) -> List[Result]:
        """Read the experiment results from json files"""
        results = []
        for vial in experiment.vials:
            try:
                results.append(
                    Result.read_from_json_file(
                        self.get_vial_dir(experiment.name, vial.uuid), vial.uuid
                    )
                )
            except FileNotFoundError:
                self.logger.warning(
                    "Json results file not found for vial %s", {vial.uuid}
                )
            except json.JSONDecodeError:
                self.logger.warning(
                    "JSON result file decoder error for vial %s", vial.uuid
                )
        return results


def get_nbatches(num_vials: int, batch_size: int) -> int:
    """Get the number of batches for the given batch size"""
    return math.ceil(float(num_vials) / float(batch_size))
