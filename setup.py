"""Setup script for pctsp."""

from cmaketools import setup

setup(
    author="Patrick O'Hara",
    author_email="patrick.h.o-hara@warwick.ac.uk",
    description="pctsp",
    install_requires=[
        "tspwplib>=0.5.7",
    ],
    name="pctsp",
    setup_requires=["setuptools_scm"],
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
    src_dir=".",
    ext_module_hint="Python3_add_library",
    has_package_data=True,
)
