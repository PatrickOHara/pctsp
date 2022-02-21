"""Constant values for vertices"""

import sys
from tspwplib import Vertex

NULL_VERTEX: Vertex = -sys.maxsize - 1
FOUR_HOURS: float = (float)(60 * 60 * 4)
LP_GAP_IMPROVEMENT_THRESHOLD = 0.001
BOOST_LOGS_TXT: str = "boost_logs.txt"
SCIP_BOUNDS_CSV: str = "lower_upper_bounds.csv"
SCIP_LOGS_TXT: str = "scip_logs.txt"
SCIP_NODE_STATS_CSV: str = "scip_node_stats.csv"
PCTSP_SUMMARY_STATS_YAML: str = "pctsp_summary_stats.yaml"
