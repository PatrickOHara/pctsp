import distutils
import os
from pathlib import Path
import site
import sys
from typing import List


def is_site_package_candidate(candidate: str) -> bool:
    if not candidate:  # candidate is empty string or none
        return False
    candidate_path = Path(candidate)
    return candidate_path.exists() and candidate_path.is_dir()


def site_packages_candidate_list() -> List[str]:
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
    candidate_list = site_packages_candidate_list()
    if len(candidate_list) == 0:
        raise FileNotFoundError("No suitable site package directories were found")
    return candidate_list[0]


def get_relative_site_package(candidate: str, relative_to: str = sys.prefix) -> str:
    return os.path.relpath(candidate, relative_to)


def get_relative_prefered_site_package(relative_to: str = sys.prefix) -> str:
    return get_relative_site_package(
        get_prefered_site_package(), relative_to=relative_to
    )


if __name__ == "__main__":
    try:
        relative_to = sys.argv[1]
    except IndexError:
        relative_to = sys.prefix
    print(get_relative_prefered_site_package(relative_to))