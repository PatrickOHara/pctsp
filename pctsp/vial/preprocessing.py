"""Settings for preprocessing graph"""

from pydantic import BaseModel

# pylint: disable=abstract-method


class Preprocessing(BaseModel):
    """Preprocessing settings"""

    disjoint_path_cutoff: bool
    remove_leaves: bool
    remove_disconnected_components: bool
    remove_one_connected_components: bool
    shortest_path_cutoff: bool
