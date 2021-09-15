"""Data structures for Prize-collecting TSP"""

from pathlib import Path
from pydantic import BaseModel
import yaml


class SummaryStats(BaseModel):

    status: int
    lower_bound: float
    upper_bound: float
    num_cost_cover_disjoint_paths: int
    num_cost_cover_shortest_paths: int
    num_nodes: int
    num_sec_disjoint_tour: int
    num_sec_maxflow_mincut: int

    @classmethod
    def from_yaml(cls, yaml_filepath: Path):
        """Get summary stats class from yaml file"""
        with open(yaml_filepath, "r", encoding="utf-8") as yaml_file:
            summary_dict = yaml.full_load(yaml_file)
            return cls(**summary_dict)
