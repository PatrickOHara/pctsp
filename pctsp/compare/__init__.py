"""Functions that return lists of vials to compare algorithms against eachother"""

from .dryrun_experiment import dryrun
from .exact_experiment import cost_cover, simple_branch_cut, tailing_off
from .heuristic_experiment import compare_heuristics, londonaq_alpha
from .relaxation_experiment import disjoint_tours_vs_heuristics

__all__ = [
    "cost_cover",
    "disjoint_tours_vs_heuristics",
    "dryrun",
    "compare_heuristics",
    "londonaq_alpha",
    "simple_branch_cut",
    "tailing_off",
]
