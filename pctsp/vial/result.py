"""Results of Vials"""

from datetime import datetime
import json
from pathlib import Path
from typing import Any, List, Tuple
from uuid import UUID
from pydantic import BaseModel


# pylint: disable=abstract-method
class Result(BaseModel):

    """Results of an experiment"""

    vial_uuid: UUID
    objective: int
    prize: int
    edge_list: List[Tuple[int, int]]
    start_time: datetime
    end_time: datetime

    def write_to_json_file(self, directory: Path) -> Path:
        """Write the result to a json file. Name of the file is the vial_uuid.

        Args:
            directory: directory to write the file.

        Returns:
            Filepath to result json file

        Raises:
            FileNotFoundError: If the directory does not exist
        """
        if not directory.exists():
            raise FileNotFoundError(f"{str(directory)} does not exist")
        filename = str(self.vial_uuid) + ".json"
        filepath = directory / filename
        with open(filepath, "w", encoding="utf-8") as json_file:
            json.dump(json.loads(self.model_dump_json()), json_file, indent=4)
        return filepath

    @classmethod
    def read_from_json_file(cls, directory: Path, vial_uuid: UUID) -> Any:
        """Read a result from a json file

        Args:
            directory: directory to read the file from
            vial_uuid: UUID of the vial (also the name of the json file)

        Returns:
            A new Result object
        """
        if not directory.exists():
            raise FileNotFoundError(f"{str(directory)} does not exist")
        filename = str(vial_uuid) + ".json"
        filepath = directory / filename
        with open(filepath, "r", encoding="utf-8") as json_file:
            return cls(**json.load(json_file))
