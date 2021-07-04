"""Tests to ensure C++ is writing logs to file"""

from pctsp import pctsp_branch_and_cut


def test_branch_and_cut_logger(suurballes_undirected_graph, root, logger_path):
    """Test the logs are saved from SCIP"""
    assert logger_path.parent.exists()
    assert not logger_path.exists()
    pctsp_branch_and_cut(suurballes_undirected_graph, 5, root, log_file=logger_path)
    assert logger_path.exists()

    # count number of lines in file. Check it is greater than zero
    line_count = 0
    with open(logger_path, "r") as log_file:
        for line in log_file:
            if line != "\n":
                line_count += 1
    assert line_count > 0
