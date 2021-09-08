cmake_minimum_required(VERSION 3.16.0)
project(pctsp_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# list of modules to find
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules;")

# not sure if this is needed?
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# root directories - recommended to set
# if (NOT Python_ROOT)
#   set(Python_ROOT $ENV{Python_ROOT})
# endif()
# if (NOT Python3_ROOT)
#   set(Python3_ROOT $ENV{Python3_ROOT})
# endif()
if (NOT CMAKE_SYSTEM_PREFIX_PATH)
  set(CMAKE_SYSTEM_PREFIX_PATH $ENV{CMAKE_SYSTEM_PREFIX_PATH})
endif()

# libraries for SCIP dependencies (optional on most systems)
# if (NOT Readline_ROOT)
#   set(Readline_ROOT $ENV{Readline_ROOT})
# endif()
# if (NOT ZLIB_ROOT)
#   set(ZLIB_ROOT $ENV{ZLIB_ROOT})
# endif()

# required dependencies
find_package(Python3 3.6 REQUIRED COMPONENTS Interpreter Development REQUIRED)
find_package(Boost 1.71.0 REQUIRED COMPONENTS python3 graph log log_setup thread filesystem system REQUIRED)
find_package(SCIP REQUIRED)

# optional dependencies - on some systems, SCIP complains if the following packages are not linked
find_package(Readline)
find_package(GMP)
find_package(ZLIB)

# find the python sitelibs
set(FindSitePkgCommand "import site; print(site.getsitepackages()[0])")
execute_process ( COMMAND ${Python3_EXECUTABLE} -c "${FindSitePkgCommand}"
    OUTPUT_VARIABLE Python3_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
# depends upon scikit-build being installed
list(APPEND CMAKE_MODULE_PATH "${Python3_SITE_PACKAGES}/skbuild/resources/cmake;")
message("cmake module path: " ${CMAKE_MODULE_PATH})

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
add_subdirectory(tests)