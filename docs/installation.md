# Installation

Install with pip:

```bash
pip install .
```

Alternatively if you only need the c++ code, you can use cmake:

```bash
mkdir build
cd build
cmake ..
cmake --build .
cmake --install .   # optional install
```

## Virtual env

If you are using a virtual env (e.g. conda) or have multiple versions of python installed,
you may want to set some environment variables.
A convenient way to do this is to create a `.env` file.
The `setup.py` scripts is configured to look for such a file and set the env variables it finds.
Here is a useful example for a conda environment called `pctsp-env`:

```
PREFIX=/opt/anaconda3/envs/pctsp-env
Python3_ROOT_DIR=${PREFIX}
Python_ROOT_DIR=${PREFIX}
CMAKE_INSTALL_PREFIX=${PREFIX}
```

Notice we have set both the `Python3_ROOT_DIR` and `Python_ROOT_DIR` to be the same.
This is because some dependencies (gtest, scikit-build) use the [FindPython](https://cmake.org/cmake/help/latest/module/FindPython.html)
cmake function instead of [FindPython3](https://cmake.org/cmake/help/latest/module/FindPython3.html?highlight=findpython3) which can cause different versions of python to be found for different dependencies.

## Dynamic linking

We dynamically link python at run-time, rather than linking at compile time.
[See this blog post](https://cmake.org/cmake/help/latest/module/FindPython3.html?highlight=findpython3) for a good explanation of our method on Mac OS X.
