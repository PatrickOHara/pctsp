"""A vial is a small glass container used in experiments"""

from typing import Any, Dict
from uuid import UUID
import pandas as pd
from pydantic import BaseModel, ConfigDict
from .data_config import DataConfig
from .model_params import ModelParams
from .preprocessing import Preprocessing

# pylint: disable=abstract-method


class Vial(BaseModel):
    """Data + Model + Preprocessing = Vail"""

    uuid: UUID
    data_config: DataConfig
    model_params: ModelParams
    preprocessing: Preprocessing

    # ignore pydantic namespaces
    model_config = ConfigDict(protected_namespaces=())


def flat_dict_from_vial(vial: Vial) -> Dict[str, Any]:
    """A flat dictionary from a vial.

    A 'flat' dict does not contain dicts as values.

    Args:
        vial: The vial to construct a flat dictionary from

    Returns:
        Dictionary of keys and values where values are not dicts
    """
    vial_dict = {"uuid": vial.uuid}
    not_flat_dict = {
        "data_config": vial.data_config.dict(),
        "model_params": vial.model_params.dict(),
        "preprocessing": vial.preprocessing.dict(),
    }
    for value_dict in not_flat_dict.values():
        for field, value in value_dict.items():
            vial_dict[field] = value
    return vial_dict


def dataframe_from_vial_list(vial_list) -> pd.DataFrame:
    """Convert a list of vials to a dataframe"""
    return pd.DataFrame(map(flat_dict_from_vial, vial_list))
