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
    bfs_extension_collapse = "bfs_extension_collapse"
    bfs_path_extension_collapse = "bfs_path_extension_collapse"
    solve_pctsp = "solve_pctsp"
    suurballes_extension_collapse = "suurballes_extension_collapse"
    suurballes_heuristic = "suurballes_heuristic"
    suurballes_path_extension_collapse = "suurballes_path_extension_collapse"


class LongAlgorithmName(StrEnumMixin, str, Enum):
    """Long, pretty names for algorithms"""

    disjoint_tours_relaxation = "Disjoint Tours Relaxation"
    extend = "Extension"
    extension = "Extension"
    bfs_extension_collapse = "BFS Extension and Collapse"
    bfs_path_extension_collapse = "BFS Path Extension and Collapse"
    solve_pctsp = "PCTSP Branch and Cut"
    suurballes_extension_collapse = "Suurballe's Extension and Collapse"
    suurballes_heuristic = "Suurballe's heuristic"
    suurballes_path_extension_collapse = "Suurballe's Path Extension and Collapse"


class ShortAlgorithmName(StrEnumMixin, str, Enum):
    """Short names for algorithms"""

    disjoint_tours_relaxation = "DTR"
    bfs_extension_collapse = "BFS-EC"
    bfs_path_extension_collapse = "BFS-PEC"
    extension = "Ex"
    solve_pctsp = "BC"
    suurballes_extension_collapse = "SBL-EC"
    suurballes_heuristic = "SBL"
    suurballes_path_extension_collapse = "SBL-PEC"


EXACT_ALGORITHMS: List[AlgorithmName] = [AlgorithmName.solve_pctsp]
HEURISTIC_ALGORITHMS: List[AlgorithmName] = [
    AlgorithmName.extension,
    AlgorithmName.bfs_extension_collapse,
    AlgorithmName.bfs_path_extension_collapse,
    AlgorithmName.suurballes_extension_collapse,
    AlgorithmName.suurballes_heuristic,
    AlgorithmName.suurballes_path_extension_collapse,
]
RELAXATION_ALGORITHMS: List[AlgorithmName] = [AlgorithmName.disjoint_tours_relaxation]


class BranchingStrategy(IntEnum):
    """Codes for branching rules"""

    RELPSCOST = 0
    STRONG = 1
    STRONG_AT_TREE_TOP = 2
    DEFAULT = 4


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
