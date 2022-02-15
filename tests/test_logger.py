"""Tests to ensure C++ is writing logs to file"""

from pyscipopt import Model
from pctsp import SCIP_LOGS_TXT, solve_pctsp


def test_branch_and_cut_logger(suurballes_undirected_graph, root, logger_dir):
    """Test the logs are saved from SCIP"""
    logger_path = logger_dir / SCIP_LOGS_TXT
    assert logger_path.parent.exists()
    assert not logger_path.exists()

    model = Model(
        problemName="test_branch_and_cut_logger", createscip=True, defaultPlugins=False
    )
    solve_pctsp(
        model,
        suurballes_undirected_graph,
        [],
        5,
        root,
        solver_dir=logger_dir,
    )
    assert logger_path.exists()

    # count number of lines in file. Check it is greater than zero
    line_count = 0
    with open(logger_path, "r", encoding="utf-8") as log_file:
        for line in log_file:
            if line != "\n":
                line_count += 1
    assert line_count > 0
