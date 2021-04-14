"""Useful functions for python packages"""

import distutils
import os
from pathlib import Path
import site
import sys
from typing import List


def is_site_package_candidate(candidate: str) -> bool:
    """Is the filepath candidate a site package directory?"""
    if not candidate:  # candidate is empty string or none
        return False
    candidate_path = Path(candidate)
    return candidate_path.exists() and candidate_path.is_dir()


def site_packages_candidate_list() -> List[str]:
    """List of site package candidates"""
    candidate_list = []
    try:
        for candidate in site.getsitepackages():
            if is_site_package_candidate(candidate):
                candidate_list.append(candidate)
    except AttributeError:
        pass
    try:
        candidate = distutils.sysconfig.get_python_lib()
        if is_site_package_candidate(candidate):
            candidate_list.append(candidate)
    except AttributeError:
        pass

    try:
        candidate = site.getusersitepackages()
        if is_site_package_candidate(candidate):
            candidate_list.append(candidate)
    except AttributeError:
        pass
    return candidate_list


def get_prefered_site_package() -> str:
    """Get the first site package in the candidate list"""
    candidate_list = site_packages_candidate_list()
    if len(candidate_list) == 0:
        raise FileNotFoundError("No suitable site package directories were found")
    return candidate_list[0]


def get_relative_site_package(candidate: str, relative: str = sys.prefix) -> str:
    """Get the relative filepath for the site package directory"""
    return os.path.relpath(candidate, relative)


def get_relative_prefered_site_package(relative: str = sys.prefix) -> str:
    """Get the relative filepath for the preferred relative site package"""
    return get_relative_site_package(get_prefered_site_package(), relative=relative)
