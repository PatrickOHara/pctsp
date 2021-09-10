"""Constant values for vertices"""

import sys
from tspwplib import Vertex

NULL_VERTEX: Vertex = -sys.maxsize - 1
FOUR_HOURS: float = (float)(60 * 60 * 4)
BOOST_LOGS_TXT: str = "boost_logs.txt"
SCIP_BOUNDS_CSV: str = "scip_bounds.csv"
SCIP_LOGS_TXT: str = "scip_logs.txt"
SCIP_METRICS_CSV: str = "scip_metrics.csv"
PCTSP_SUMMARY_STATS_YAML: str = "summary_stats.yaml"
