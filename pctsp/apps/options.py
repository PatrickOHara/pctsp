"""Options for the command line interface"""

import logging
from typer import Option
from tspwplib import (
    Alpha,
    Generation,
    GraphName,
    LondonaqGraphName,
)
from ..constants import FOUR_HOURS

# dataset settings
AlphaOption = Option(
    Alpha.fifty.value, help="Percent of the total prize to set the quota"
)
EdgeRemovalProbability = Option(
    0.0, help="Probability of removing an edge to increase sparsity"
)
GenerationOption = Option(Generation.gen1, help="Generation of the OPLib instance")
GraphNameOption = Option(GraphName.eil76, help="Graph instance name of tsplib95")
LondonaqGraphNameOption = Option(
    LondonaqGraphName.laqtinyA, help="Name of londonaq graph"
)

# heuristic settings
CollapsePathsOption = Option(False, help="Collapse shortest paths")
PathDepthLimitOption = Option(2, help="Depth of path search for extension")
StepSizeOption = Option(1, help="Size of step during extension")


# preprocessing settings
RemoveLeavesOption = Option(False, help="Remove leaves from the graph")

# algorithm settings
TimeLimitOption = Option(FOUR_HOURS, help="Stop algorithm after time limit")

# misc
LabDirOption = Option(
    ...,
    envvar="LAB_DIR",
    resolve_path=True,
    file_okay=False,
    dir_okay=True,
    readable=True,
    writable=True,
    exists=False,
)
LondonaqRootOption = Option(
    ...,
    envvar="LONDONAQ_ROOT",
    resolve_path=True,
    file_okay=False,
    dir_okay=True,
    readable=True,
    exists=True,
)
OPLibRootOption = Option(
    ...,
    envvar="OPLIB_ROOT",
    resolve_path=True,
    file_okay=False,
    dir_okay=True,
    readable=True,
    exists=True,
)
SecretFileOption = Option(
    ".secrets.yml",
    envvar="PCTSP_SECRETS",
    help="Filepath to secrets yml",
    resolve_path=True,
    file_okay=True,
    dir_okay=False,
    readable=True,
    exists=False,
)
LoggingLevelOption = Option(logging.INFO, help="Set the logging level")
