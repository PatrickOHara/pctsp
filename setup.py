"""Setup script for pctsp."""

from skbuild import setup

setup(
    author="Patrick O'Hara",
    author_email="patrick.h.o-hara@warwick.ac.uk",
    description="pctsp",
    install_requires=[
        "tspwplib>=0.5.7",
    ],
    name="pctsp",
    setup_requires=["setuptools-scm"],
    packages=["pctsp"],
    python_requires=">=3.6",
    cmake_source_dir=".",
)

# import os
# import pathlib

# from setuptools import setup, Extension
# from setuptools.command.build_ext import build_ext as build_ext_orig


# class CMakeExtension(Extension):
#     def __init__(self, name):
#         # don't invoke the original build_ext for this special extension
#         super().__init__(name, sources=[])


# class build_ext(build_ext_orig):
#     def run(self):
#         for ext in self.extensions:
#             ext.cython_directives = {"language_level": "3"}
#             self.build_cmake(ext)
#         super().run()

#     def build_cmake(self, ext):
#         cwd = pathlib.Path().absolute()

#         # these dirs will be created in build_py, so if you don't have
#         # any python sources to bundle, the dirs will be missing
#         build_temp = pathlib.Path(self.build_temp)
#         build_temp.mkdir(parents=True, exist_ok=True)
#         extdir = pathlib.Path(self.get_ext_fullpath(ext.name))
#         extdir.mkdir(parents=True, exist_ok=True)

#         # example of cmake args
#         config = "Debug" if self.debug else "Release"
#         cmake_args = [
#             "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=" + str(extdir.parent.absolute()),
#             "-DCMAKE_BUILD_TYPE=" + config,
#         ]

#         # example of build args
#         build_args = ["--config", config, "--", "-j4"]

#         os.chdir(str(build_temp))
#         self.spawn(["cmake", str(cwd)] + cmake_args)
#         if not self.dry_run:
#             self.spawn(["cmake", "--build", "."] + build_args)
#         # Troubleshooting: if fail on line above then delete all possible
#         # temporary CMake files including "CMakeCache.txt" in top level dir.
#         os.chdir(str(cwd))


# setup(
#     author="Patrick O'Hara",
#     author_email="patrick.h.o-hara@warwick.ac.uk",
#     description="pctsp",
#     install_requires=[
#         "tspwplib>=0.5.7",
#     ],
#     name="pctsp",
#     setup_requires=["setuptools-scm"],
#     packages=["pctsp"],
#     python_requires=">=3.6",
#     use_scm_version=True,
#     license="MIT License",
#     classifiers=[
#         "Programming Language :: Python",
#         "Programming Language :: Python :: 3",
#         "Programming Language :: Python :: 3.6",
#         "Programming Language :: Python :: 3.7",
#         "Programming Language :: Python :: 3.8",
#         "Programming Language :: Python :: 3 :: Only",
#         "Operating System :: OS Independent",
#     ],
#     ext_modules=[CMakeExtension("pctsp/libpctsp")],
#     cmdclass={
#         "build_ext": build_ext,
#     },
# )
