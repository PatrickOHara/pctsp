"""The Lab we run experiment inside"""

from .lab import Lab, get_nbatches, EXPERIMENT_FILENAME, RESULT_FILENAME
from .run_algorithm import run_algorithm, run_heuristic, run_preprocessing

__all__ = [
    "Lab",
    "get_nbatches",
    "run_algorithm",
    "run_heuristic",
    "run_preprocessing",
    "EXPERIMENT_FILENAME",
    "RESULT_FILENAME",
]
