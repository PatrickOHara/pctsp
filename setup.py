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
        "tspwplib>=0.6.1",
    ],
    name="pctsp",
    packages=["pctsp"],
    python_requires=">=3.6",
    use_scm_version=True,
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
    cmake_args=['-DPCTSP_BUILD_TESTS:BOOL=OFF'],
    cmake_source_dir=".",
)
