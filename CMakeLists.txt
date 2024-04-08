cmake_minimum_required(VERSION 3.27)
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
find_package(Python 3.12 REQUIRED COMPONENTS Interpreter Development REQUIRED)
find_package(Boost 1.84.0 REQUIRED COMPONENTS graph log log_setup thread filesystem system REQUIRED)
find_package(SCIP 9.0.0 REQUIRED)
find_package(YAML_CPP REQUIRED FOUND_VAR YAML_CPP_FOUND)
find_package(pybind11 CONFIG HINTS ${Python_SITELIB} PATH_SUFFIXES "pybind11")

# depends upon scikit-build being installed
list(APPEND CMAKE_MODULE_PATH "${Python_SITELIB}/skbuild/resources/cmake;")

set(PCTSP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
set(MACOSX_RPATH TRUE)
endif()

add_subdirectory(src)
add_subdirectory(pctsp)
if (${PCTSP_BUILD_TESTS})
  add_subdirectory(tests)
endif()