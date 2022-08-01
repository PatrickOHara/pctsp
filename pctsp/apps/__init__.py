"""Apps for the Prize-collecting TSP application"""

from .main_app import app, lab, tsplib
from .tables_app import pretty_dataframe

__all__ = ["app", "lab", "pretty_dataframe", "tsplib"]
