"""Test the commands that make up the tspwp CLI app"""


import logging
import pandas as pd
from tspwplib import Generation, GraphName
from pctsp.apps import lab, pretty_dataframe, tsplib
from pctsp.lab import EXPERIMENT_FILENAME, RESULT_FILENAME
from pctsp.vial import AlgorithmName, LongAlgorithmName, DatasetName, ExperimentName


def test_pretty_dataframe() -> None:
    """Test dataframes are made pretty"""
    df = pd.DataFrame(
        {"algorithm": [AlgorithmName.solve_pctsp.value], "duration": [0.551]},
    )
    df = df.set_index("algorithm")
    pretty_df = pretty_dataframe(df, long=True)
    assert (
        LongAlgorithmName.solve_pctsp.value
        in pretty_df.index.get_level_values("Algorithm").values
    )
    assert "TIME" in pretty_df


def test_lab_command(lab_dir, oplib_root):
    """Test the lab entrypoint command"""
    test_experiment = ExperimentName.dryrun
    lab(
        DatasetName.tspwplib,
        test_experiment,
        lab_dir=lab_dir,
        logging_level=logging.DEBUG,
        oplib_root=oplib_root,
    )
    experiment_dir = lab_dir / DatasetName.tspwplib.value / test_experiment.value
    assert (experiment_dir / EXPERIMENT_FILENAME).exists()
    assert (experiment_dir / RESULT_FILENAME).exists()


def test_tsplib_command(algorithm_name, lab_dir, oplib_root):
    """Test we can run one algorithm from the command line"""
    tsplib(
        algorithm_name,
        alpha=50,
        collapse_paths=False,
        generation=Generation.gen1,
        graph_name=GraphName.st70,
        lab_dir=lab_dir,
        path_depth_limit=2,
        remove_leaves=False,
        oplib_root=oplib_root,
        logging_level=logging.DEBUG,
        step_size=1,
        time_limit=60.0,
    )
