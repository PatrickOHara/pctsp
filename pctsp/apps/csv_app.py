"""App for creating CSV files from experiment data"""

from pathlib import Path
from typing import List
from uuid import UUID
import yaml

import numpy as np
import pandas as pd
import typer

from ..constants import PCTSP_SUMMARY_STATS_YAML
from ..data_structures import SummaryStats
from ..lab import Lab
from ..vial import (
    DatasetName,
    ExperimentName,
    Experiment,
    Result,
    dataframe_from_vial_list,
)

from .options import LabDirOption

csv_app = typer.Typer(name="csv", help="Write experiment data to CSV")


def experiment_summary_df(
    experiment: Experiment, results: List[Result]
) -> pd.DataFrame:
    """Get a dataframe summarizing the results of an experiment"""
    vial_df = dataframe_from_vial_list(experiment.vials)
    result_df = pd.DataFrame(map(lambda x: x.dict(), results))
    joined_df = vial_df.merge(result_df, left_on="uuid", right_on="vial_uuid")
    joined_df["duration"] = (joined_df["end_time"] - joined_df["start_time"]).apply(
        lambda x: x.total_seconds()
    )
    joined_df["feasible"] = joined_df["prize"] >= joined_df["quota"]

    # fill with NaNs whereever a non-feasible solution is found
    joined_df["objective"] = np.where(
        joined_df["feasible"], joined_df["objective"], np.nan
    )
    joined_df["overshoot"] = np.where(
        joined_df["feasible"], joined_df["prize"] - joined_df["quota"], np.nan
    )
    return joined_df


def summary_stats_df(root_dir: Path, ids: List[UUID]) -> pd.DataFrame:
    """Get a dataframe of summary stats for each of the IDS"""
    stats_list = []
    for vial_uuid in ids:
        stats_path = root_dir / str(vial_uuid) / PCTSP_SUMMARY_STATS_YAML
        try:
            stats = SummaryStats.from_yaml(stats_path)
            stats_dict = stats.dict()
            stats_dict["vial_uuid"] = vial_uuid
            stats_list.append(stats_dict)
        except (FileNotFoundError, yaml.YAMLError):
            pass
    return pd.DataFrame.from_records(stats_list)


@csv_app.command(name="heuristics")
def write_heuristics_experiment_to_csv(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    lab_dir: Path = LabDirOption,
) -> None:
    """Load the experiment, summerize in a dataframe then write to CSV"""
    stats_lab = Lab(lab_dir / dataset.value)
    experiment = stats_lab.read_experiment_from_file(experiment_name)
    results = stats_lab.read_results_from_file(experiment)
    heuristics_df = experiment_summary_df(experiment, results)
    print(heuristics_df)
    filename = (
        stats_lab.get_experiment_dir(experiment_name)
        / f"{dataset.value}_{experiment_name.value}.csv"
    )
    heuristics_df.to_csv(filename)


@csv_app.command(name="branch-cut")
def write_branch_cut_experiment_to_csv(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    lab_dir: Path = LabDirOption,
) -> None:
    """Write a branch and cut experiment to CSV"""
    stats_lab = Lab(lab_dir / dataset.value)
    experiment = stats_lab.read_experiment_from_file(experiment_name)
    results = stats_lab.read_results_from_file(experiment)
    experiment_df = experiment_summary_df(experiment, results)  # cost cover dataframe
    summary_df = summary_stats_df(
        stats_lab.get_experiment_dir(experiment_name),
        [vial.uuid for vial in experiment.vials],
    )
    experiment_df = experiment_df.merge(
        summary_df, left_on="vial_uuid", right_on="vial_uuid"
    )
    filename = (
        stats_lab.get_experiment_dir(experiment_name)
        / f"{dataset.value}_{experiment_name.value}.csv"
    )
    experiment_df.to_csv(filename)
