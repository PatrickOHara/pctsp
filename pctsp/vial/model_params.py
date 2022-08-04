"""Parameters for a model or algorithm"""

from enum import Enum, IntEnum
from typing import List, Optional
from pydantic import BaseModel
from tspwplib.types import StrEnumMixin

# pylint: disable=invalid-name
class AlgorithmName(StrEnumMixin, str, Enum):
    """Names of valid algorithms for PCTSP"""

    disjoint_tours_relaxation = "disjoint_tours_relaxation"
    extension = "extension"
    extension_collapse = "extension_collapse"
    path_extension_collapse = "path_extension_collapse"
    solve_pctsp = "solve_pctsp"
    suurballes_heuristic = "suurballes_heuristic"


class LongAlgorithmName(StrEnumMixin, str, Enum):
    """Long, pretty names for algorithms"""

    disjoint_tours_relaxation = "Disjoint Tours Relaxation"
    extend = "Extension"
    extension = "Extension"
    extension_collapse = "Extension and Collapse"
    path_extension_collapse = "Path Extension and Collapse"
    solve_pctsp = "PCTSP Branch and Cut"
    suurballes_heuristic = "Suurballe's heuristic"


class ShortAlgorithmName(StrEnumMixin, str, Enum):
    """Short names for algorithms"""

    disjoint_tours_relaxation = "DTR"
    extension = "Ex"
    extension_collapse = "EC"
    path_extension_collapse = "PEC"
    solve_pctsp = "BC"
    suurballes_heuristic = "SH"


EXACT_ALGORITHMS: List[AlgorithmName] = [AlgorithmName.solve_pctsp]
HEURISTIC_ALGORITHMS: List[AlgorithmName] = [
    AlgorithmName.extension,
    AlgorithmName.extension_collapse,
    AlgorithmName.path_extension_collapse,
    AlgorithmName.suurballes_heuristic,
]
RELAXATION_ALGORITHMS: List[AlgorithmName] = [AlgorithmName.disjoint_tours_relaxation]


class BranchingStrategy(IntEnum):
    """Codes for branching rules"""

    RELPSCOST = 0
    STRONG = 1
    STRONG_AT_TREE_TOP = 2


# pylint: disable=abstract-method,too-many-instance-attributes
class ModelParams(BaseModel):
    """Parameters of an algorithm or model"""

    algorithm: AlgorithmName
    is_exact: bool
    is_heuristic: bool
    is_relaxation: bool
    branching_strategy: Optional[BranchingStrategy]
    branching_max_depth: Optional[int]
    collapse_paths: Optional[bool]
    cost_cover_disjoint_paths: Optional[bool]
    cost_cover_shortest_path: Optional[bool]
    heuristic: Optional[AlgorithmName]
    path_depth_limit: Optional[int]
    sec_disjoint_tour: Optional[bool]
    sec_lp_gap_improvement_threshold: Optional[float]
    sec_maxflow_mincut: Optional[bool]
    sec_max_tailing_off_iterations: Optional[int]
    sec_sepafreq: Optional[int]
    step_size: Optional[int]
    time_limit: Optional[float]
