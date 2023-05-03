"""Shared lists of parameters and constants across experiments"""

from typing import List, Optional
from tspwplib import EdgeWeightType, LondonaqGraphName
from tspwplib.types import GraphName

LONDONAQ_QUOTA_LIST: List[int] = [1000, 2000, 3000, 4000, 5000, 6000]
LONDONAQ_GRAPH_NAME_LIST: List[LondonaqGraphName] = [
    LondonaqGraphName.laqbbA,
    LondonaqGraphName.laqidA,
    LondonaqGraphName.laqkxA,
    LondonaqGraphName.laqwsA,
]

TSPLIB_ALPHA_LIST: List[int] = [5, 10, 25, 50, 75]
TSPLIB_COST_FUNCTIONS: List[EdgeWeightType] = [
    EdgeWeightType.EUC_2D,
    EdgeWeightType.MST,
]
TSPLIB_KAPPA_LIST: List[Optional[int]] = [5, 10, 15, 20, 25]
TSPLIB_GRAPH_NAME_LIST: List[Optional[GraphName]] = [
    GraphName.eil51,
    GraphName.st70,
    GraphName.rat195,
    GraphName.tsp225,
    GraphName.a280,
    GraphName.pr439,
]
