"""Setup script for pctsp."""

from dotenv import load_dotenv
from skbuild import setup
from setuptools import find_packages

load_dotenv()

setup(
    author="Patrick O'Hara",
    author_email="patrick.h.o-hara@warwick.ac.uk",
    description="Algorithms for the Prize-collecting Travelling Salesperson Problem",
    install_requires=[
        "colorlog>=4.6.2",
        "plotly>=4.14.1",
        "pydantic>=1.8.0",
        "pyscipopt>=4.0.0",
        "tspwplib@git+https://github.com/PatrickOHara/tspwplib.git@main",
        "typer>=0.4.0",
    ],
    name="pctsp",
    packages=find_packages(),
    python_requires=">=3.12",
    use_scm_version={"fallback_version": "1.1.0"},
    license="MIT License",
    classifiers=[
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3 :: Only",
        "Operating System :: OS Independent",
    ],
    cmake_args=[
        "-GNinja",
        "-DPCTSP_BUILD_TESTS:BOOL=OFF",
    ],
    cmake_source_dir=".",
    entry_points={"console_scripts":["pctsp=pctsp.__main__:app"]},
)
