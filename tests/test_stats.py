"""Make sure stats from solver can be read by python"""

# NOTE For now this test is turned off

# import pandas as pd
# from pyscipopt import Model
# from pctsp import SCIP_BOUNDS_CSV, SCIP_NODE_STATS_CSV, solve_pctsp


# def test_read_node_stats(suurballes_undirected_graph, root, stats_dir):
#     """Test reading stats from csv"""
#     model = Model(
#         problemName="test_read_node_stats", createscip=True, defaultPlugins=False
#     )
#     solve_pctsp(
#         model,
#         suurballes_undirected_graph,
#         [],
#         6,
#         root,
#         solver_dir=stats_dir,
#     )
#     filepath = stats_dir / SCIP_NODE_STATS_CSV
#     assert filepath.exists()
#     stats_df = pd.read_csv(filepath)
#     assert "node_id" in stats_df
#     assert "lower_bound" in stats_df

#     bounds_filepath = stats_dir / SCIP_BOUNDS_CSV
#     assert bounds_filepath.exists()
#     bounds_df = pd.read_csv(bounds_filepath)
#     assert len(bounds_df) > 0
#     time_cols = ["start_timestamp", "end_timestamp"]
#     bound_cols = ["lower_bound", "upper_bound", "node_id"]
#     for col in time_cols + bound_cols:
#         assert col in bounds_df
#     for col in time_cols:
#         bounds_df[col] = pd.to_datetime(bounds_df[col])
#     for _, row in bounds_df[time_cols].iterrows():
#         assert row["start_timestamp"] <= row["end_timestamp"]
