"""App for useful functions"""

from pathlib import Path
from shutil import make_archive, unpack_archive
from typing import List
import typer
from ..vial import DatasetName, ExperimentName
from .options import LabDirOption

utils_app = typer.Typer(name="utils", help="Useful functions")


@utils_app.command(name="zip")
def zip_experiments(
    dataset: DatasetName,
    experiment_name_list: List[ExperimentName],
    lab_dir: Path = LabDirOption,
) -> None:
    """Zip each experiment for the dataset"""
    # for dataset in dataset_list:
    root_dir = lab_dir / dataset.value
    for experiment_name in experiment_name_list:
        base_name = root_dir / experiment_name.value
        make_archive(
            str(base_name), "zip", root_dir=root_dir, base_dir=experiment_name.value
        )


@utils_app.command(name="unzip")
def unzip_experiments(
    dataset: DatasetName,
    experiment_name_list: List[ExperimentName],
    lab_dir: Path = LabDirOption,
) -> None:
    """Unzip each experiment for the dataset"""
    root_dir = lab_dir / dataset.value
    for experiment_name in experiment_name_list:
        base_name = root_dir / f"{experiment_name.value}.zip"
        unpack_archive(base_name, extract_dir=root_dir, format="zip")
