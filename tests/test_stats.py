"""Make sure stats from solver can be read by python"""

import pandas as pd
from pctsp import pctsp_branch_and_cut


def test_read_node_stats(suurballes_undirected_graph, root, stats_dir):
    """Test reading stats from csv"""
    pctsp_branch_and_cut(suurballes_undirected_graph, 6, root, output_dir=stats_dir)
    filepath = stats_dir / "node_stats.csv"
    assert filepath.exists()
    stats_df = pd.read_csv(filepath)
    assert "lower_bound" in stats_df
    assert "node_id" in stats_df
