"""Make sure stats from solver can be read by python"""

import pandas as pd
from pctsp import pctsp_branch_and_cut


def test_read_node_stats(
    suurballes_undirected_graph, root, stats_dir, bounds_filename, metrics_filename
):
    """Test reading stats from csv"""
    pctsp_branch_and_cut(
        suurballes_undirected_graph,
        6,
        root,
        bounds_csv_filename=bounds_filename,
        output_dir=stats_dir,
        metrics_filename=metrics_filename,
    )
    filepath = stats_dir / metrics_filename
    assert filepath.exists()
    stats_df = pd.read_csv(filepath)
    assert "node_id" in stats_df
    assert "lower_bound" in stats_df

    bounds_filepath = stats_dir / bounds_filename
    assert bounds_filepath.exists()
    bounds_df = pd.read_csv(bounds_filepath)
    assert len(bounds_df) > 0
    time_cols = ["start_timestamp", "end_timestamp"]
    bound_cols = ["lower_bound", "upper_bound", "node_id"]
    for col in time_cols + bound_cols:
        assert col in bounds_df
    for col in time_cols:
        bounds_df[col] = pd.to_datetime(bounds_df[col])
    for _, row in bounds_df[time_cols].iterrows():
        assert row["start_timestamp"] <= row["end_timestamp"]
