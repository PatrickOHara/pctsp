"""Setup script for pctsp."""

from dotenv import load_dotenv
from skbuild import setup

load_dotenv()

setup(
    author="Patrick O'Hara",
    author_email="patrick.h.o-hara@warwick.ac.uk",
    description="Algorithms for the Prize-collecting Travelling Salesperson Problem",
    install_requires=[
        "plotly>=4.14.1",
        "pydantic>=1.8.0",
        "pyscipopt>=4.0.0",
        "tspwplib>=0.7.2",
        "typer>=0.4.0",
    ],
    name="pctsp",
    packages=["pctsp"],
    python_requires=">=3.8",
    use_scm_version={"fallback_version": "1.0.0"},
    license="MIT License",
    classifiers=[
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3 :: Only",
        "Operating System :: OS Independent",
    ],
    cmake_args=['-DPCTSP_BUILD_TESTS:BOOL=OFF'],
    cmake_source_dir=".",
    scripts={"cli/pctsp"},
)
