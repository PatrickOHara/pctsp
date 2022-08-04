# Installation

We recommend using the provided docker images for quick and easy use of our code.
However, if you require a local installation, you have been warned it is not always straight forward.

## Docker

Pull the docker image:

```bash
docker pull patrickohara/pctsp:latest
```

## Dependencies

Installation requires [CMake](https://cmake.org/), [Ninja](https://ninja-build.org/), a compiler with [support for C++ 17 or later](https://en.cppreference.com/w/cpp/compiler_support) and python 3.8 or later.

There are three C++ library dependencies:
- [Boost](https://www.boost.org/) for graph algorithms, logging and bimaps.
- [SCIP](https://www.scipopt.org/) for solving (linear) integer programs.
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) for writing yaml files.

Each of these dependencies must be installed before the pctsp python module can be installed.
See the dependency webpages for installation instructions.
See an example of how to install the dependencies in the `scip.dockerfile`.

## C++ install

If you only need the c++ code (and tests), you can use cmake:

```bash
mkdir build
cd build
cmake -GNinja ..    # tests can be turned off with -DPCTSP_BUILD_TESTS:BOOL=OFF
cmake --build .     # build everything
cmake --install .   # optional install
```

## Python install

Install with pip. We strongly recommend *pip version 22 or later* [due to this issue](https://github.com/pypa/pip/issues/7555):

```bash
pip install .
```

## Virtual env / non-standard install

If you are using a virtual env (e.g. conda), have multiple versions of python installed,
or have dependencies stored in non-standard locations,
you may want to set some environment variables.

For example, if cmake cannot find a dependency such as scip, we can set the `SCIP_ROOT` variable when configuring:
```bash
cmake -GNinja -DSCIP_ROOT=/path/to/scip/root ..
```

If installing with pip, a convenient way to do this is to create a `.env` file.
The `setup.py` scripts is configured to look for such a file and set the env variables it finds.
Here is a useful example for a conda environment called `pctsp-env`:

```
PREFIX=/opt/anaconda3/envs/pctsp-env
SCIP_ROOT=${PREFIX}
Python3_ROOT=${PREFIX}
Python_ROOT=${PREFIX}
```

Notice we have set both the `Python3_ROOT` and `Python_ROOT` to be the same.
This is because some dependencies (gtest, scikit-build) use the [FindPython](https://cmake.org/cmake/help/latest/module/FindPython.html)
cmake function instead of [FindPython3](https://cmake.org/cmake/help/latest/module/FindPython3.html?highlight=findpython3) which can cause different versions of python to be found for different dependencies.

## Dynamic linking (Mac OS X problems)

We dynamically link python at run-time, rather than linking at compile time.

If you have dependencies installed in non-standard locations (e.g. conda), it may be necessary to "nudge" the linker to look in the right directory for dependency libraries.
For example, suppose the path to the `lib` directory of your dependency is stored in variable `PREFIX`.
By setting the `DYLD_FALLBACK_LIBRARY_PATH` environment variable, we can nudge the dynamic linker on Mac OS X to check the `PREFIX` directory:
```bash
export DYLD_FALLBACK_LIBRARY_PATH=${PREFIX}/lib
```
