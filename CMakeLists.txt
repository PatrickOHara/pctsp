cmake_minimum_required(VERSION 3.16.0)
project(pctsp)

# list of modules to find
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
message("cmake module path: " ${CMAKE_MODULE_PATH})

# dependencies
find_package(Python3 3.6 REQUIRED COMPONENTS Interpreter Development REQUIRED)
find_package(Boost 1.71.0 REQUIRED COMPONENTS python graph REQUIRED)
find_package(SCIP REQUIRED)


set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PCTSP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PCTSP_LIBRARIES "")

message("Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
message("Python3_LIBRARIES: ${Python3_LIBRARIES}")
message("SCIP_INCLUDE_DIRS: " ${SCIP_INCLUDE_DIRS})
message("SCIP_LIBARIES: " ${SCIP_LIBRARIES})
message("Boost_INCLUDE_DIR: " ${Boost_INCLUDE_DIRS})
message("Boost_LIBRARIES: " ${Boost_LIBRARIES})

# when installing directly in CMake, redirect its intall root to dist dir
# if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#     set (CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/dist" CACHE PATH "default install path" FORCE )
# endif()

# get the suffix for python extension module and store in PYTHON_MODULE_EXTENSION
execute_process (COMMAND "${Python3_EXECUTABLE}" -c "from distutils import sysconfig;print(sysconfig.get_config_var('EXT_SUFFIX'))"
RESULT_VARIABLE _result
OUTPUT_VARIABLE PYTHON_MODULE_EXTENSION
ERROR_QUIET
OUTPUT_STRIP_TRAILING_WHITESPACE)

set(BUILD_SHARED_LIBS TRUE)

add_subdirectory(pctsp)
add_subdirectory(tests)