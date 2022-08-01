"""Logging for pctsp"""

import logging
from pathlib import Path
import colorlog


def get_pctsp_logger(name: str, level: int = logging.INFO) -> logging.Logger:
    """A custom logger"""
    # create logger
    logger = logging.getLogger(name)
    logger.setLevel(level)

    # create console handler and set level to debug
    console_handler = logging.StreamHandler()
    console_handler.setLevel(level)

    # create formatter
    formatter = colorlog.ColoredFormatter(
        r"%(asctime)s %(log_color)s%(levelname)8s%(reset)s: %(message)s",
        datefmt=r"%Y-%m-%d %H:%M:%S",
        log_colors={
            "DEBUG": "green",
            "INFO": "cyan",
            "WARNING": "yellow",
            "ERROR": "red",
            "CRITICAL": "bold_red",
        },
    )
    # add formatter to ch
    console_handler.setFormatter(formatter)

    # add ch to logger
    logger.addHandler(console_handler)

    return logger


def get_scip_log_filepath(
    log_dir: Path, log_filename: str, log_extension: str = "txt"
) -> Path:
    """Get the path to the log file"""
    log_dir.mkdir(exist_ok=True, parents=False)
    return log_dir / (log_filename + "." + log_extension)
