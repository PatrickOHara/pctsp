"""Setting for datasets"""

from enum import Enum
from typing import Optional, Union
from pydantic import BaseModel
from tspwplib import Generation, GraphName
from tspwplib.types import EdgeWeightType, LondonaqGraphName, StrEnumMixin


class DatasetName(StrEnumMixin, str, Enum):
    """Names of datasets"""

    # pylint: disable=invalid-name
    londonaq: str = "londonaq"
    tspwplib: str = "tspwplib"


# pylint: disable=abstract-method


class DataConfig(BaseModel):
    """Dataset settings"""

    dataset: DatasetName
    cost_function: EdgeWeightType
    graph_name: Union[GraphName, LondonaqGraphName]
    quota: int
    root: int
    alpha: Optional[int]
    generation: Optional[Generation]
    kappa: Optional[int]
    triangle: Optional[int]
