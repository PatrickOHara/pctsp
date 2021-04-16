cmake_minimum_required(VERSION 3.16.0)
project(pctsp_project)

# list of modules to find
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules;")

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(Python_ROOT_DIR $ENV{Python3_ROOT_DIR})
set(Python3_ROOT_DIR $ENV{Python3_ROOT_DIR})
set(CMAKE_SYSTEM_PREFIX_PATH $ENV{CMAKE_SYSTEM_PREFIX_PATH})
message("CMAKE_SYSTEM_PREFIX_PATH: " ${CMAKE_SYSTEM_PREFIX_PATH})
message("Python_ROOT_DIR: " ${Python_ROOT_DIR})
message("Python3_ROOT_DIR: " ${Python3_ROOT_DIR})
message("SCIP_ROOT: " $ENV{SCIP_ROOT})

# dependencies
find_package(Python3 3.6 REQUIRED COMPONENTS Interpreter Development REQUIRED)
find_package(Boost 1.71.0 REQUIRED COMPONENTS python graph REQUIRED)
find_package(SCIP REQUIRED)

# find the python sitelibs
set(FindSitePkgCommand "import site; print(site.getsitepackages()[0])")
execute_process ( COMMAND ${Python3_EXECUTABLE} -c "${FindSitePkgCommand}"
    OUTPUT_VARIABLE Python3_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

message("Python3_INCLUDE_DIRS: " ${Python3_INCLUDE_DIRS})
message("Python3_LIBRARIES: " ${Python3_LIBRARIES})
message("Python3_SITE_PACKAGES: " ${Python3_SITE_PACKAGES})
message("SCIP_INCLUDE_DIRS: " ${SCIP_INCLUDE_DIRS})
message("SCIP_LIBARIES: " ${SCIP_LIBRARIES})
message("Boost_INCLUDE_DIR: " ${Boost_INCLUDE_DIRS})
message("Boost_LIBRARIES: " ${Boost_LIBRARIES})

set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
# depends upon scikit-build being installed
list(APPEND CMAKE_MODULE_PATH "${Python3_SITE_PACKAGES}/skbuild/resources/cmake;")
message("cmake module path: " ${CMAKE_MODULE_PATH})


if (NOT Python_ROOT_DIR)
  message(WARNING "Python_ROOT_DIR is not set. PythonExtension may use the wrong python version")
endif()
find_package(PythonExtensions REQUIRED)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PCTSP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PCTSP_LIBRARIES "")
set(MACOSX_RPATH TRUE)

# get the suffix for python extension module and store in PYTHON_MODULE_EXTENSION
execute_process (COMMAND "${Python3_EXECUTABLE}" -c "from distutils import sysconfig;print(sysconfig.get_config_var('EXT_SUFFIX'))"
RESULT_VARIABLE _result
OUTPUT_VARIABLE PYTHON_MODULE_EXTENSION
ERROR_QUIET
OUTPUT_STRIP_TRAILING_WHITESPACE)
message("PYTHON_MODULE_EXTENSION: " ${PYTHON_MODULE_EXTENSION})

add_subdirectory(src)
add_subdirectory(pctsp)
add_subdirectory(tests)