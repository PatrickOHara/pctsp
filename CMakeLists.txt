cmake_minimum_required(VERSION 3.16.0)
project(pctsp_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# list of modules to find
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules;")

# needed for dynamic lookup when installing
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (NOT CMAKE_SYSTEM_PREFIX_PATH)
  set(CMAKE_SYSTEM_PREFIX_PATH $ENV{CMAKE_SYSTEM_PREFIX_PATH})
endif()

# option to turn of building the tests
option(PCTSP_BUILD_TESTS "Turn on/off building the tests." ON)

# required dependencies
find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter Development REQUIRED)
find_package(Boost 1.74.0 REQUIRED COMPONENTS graph log log_setup thread filesystem system REQUIRED)
find_package(SCIP 8.0.0 REQUIRED)
find_package(YAML_CPP REQUIRED FOUND_VAR YAML_CPP_FOUND)

# find the python sitelibs
set(FindSitePkgCommand "import site; print(site.getsitepackages()[0])")
execute_process ( COMMAND ${Python3_EXECUTABLE} -c "${FindSitePkgCommand}"
    OUTPUT_VARIABLE Python3_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
# depends upon scikit-build being installed
list(APPEND CMAKE_MODULE_PATH "${Python3_SITE_PACKAGES}/skbuild/resources/cmake;")

message("pybind11_DIR: " $ENV{pybind11_DIR})
find_package(pybind11 CONFIG HINTS ${Python3_SITE_PACKAGES} PATH_SUFFIXES "pybind11")
find_package(PythonExtensions REQUIRED)

set(PCTSP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
set(MACOSX_RPATH TRUE)
endif()

# get the suffix for python extension module and store in PYTHON_MODULE_EXTENSION
execute_process (COMMAND "${Python3_EXECUTABLE}" -c "from distutils import sysconfig;print(sysconfig.get_config_var('EXT_SUFFIX'))"
RESULT_VARIABLE _result
OUTPUT_VARIABLE PYTHON_MODULE_EXTENSION
ERROR_QUIET
OUTPUT_STRIP_TRAILING_WHITESPACE)
message("PYTHON_MODULE_EXTENSION: " ${PYTHON_MODULE_EXTENSION})

add_subdirectory(src)
add_subdirectory(pctsp)
if (${PCTSP_BUILD_TESTS})
  add_subdirectory(tests)
endif()