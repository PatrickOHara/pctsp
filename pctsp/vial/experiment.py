"""Classes for experiments"""

from datetime import datetime
from enum import Enum
from typing import List
from pydantic import BaseModel
from .vial import Vial


class ExperimentName(str, Enum):
    """Valid experiment names"""

    # pylint: disable=invalid-name

    cost_cover = "cost_cover"
    disjoint_tours_vs_heuristics = "disjoint_tours_vs_heuristics"
    dryrun = "dryrun"
    compare_heuristics = "compare_heuristics"
    onerun = "onerun"
    simple_branch_cut = "simple_branch_cut"
    tailing_off = "tailing_off"


# pylint: disable=abstract-method


class Experiment(BaseModel):
    """An experiment is a collection of vials"""

    name: ExperimentName
    vials: List[Vial]
    timestamp: datetime
