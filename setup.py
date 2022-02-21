"""Setup script for pctsp."""

from dotenv import load_dotenv
from skbuild import setup

load_dotenv()

setup(
    author="Patrick O'Hara",
    author_email="patrick.h.o-hara@warwick.ac.uk",
    description="pctsp",
    install_requires=[
        "pydantic>=1.8.0",
        "pyscipopt>=3.3.0",
        "tspwplib>=0.6.6",
    ],
    name="pctsp",
    packages=["pctsp"],
    python_requires=">=3.6",
    use_scm_version={"fallback_version": "0.1.7"},
    license="MIT License",
    classifiers=[
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3 :: Only",
        "Operating System :: OS Independent",
    ],
    # TODO remove debug flag
    cmake_args=['-DPCTSP_BUILD_TESTS:BOOL=OFF', '-DCMAKE_BUILD_TYPE=Debug'],
    cmake_source_dir=".",
)
