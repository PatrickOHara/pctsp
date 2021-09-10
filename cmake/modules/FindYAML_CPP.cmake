# - Try to find yaml-cpp
# Once done this will define
#   YAML_CPP_INCLUDE_DIRS   - yaml-cpp header files
#   YAML_CPP_LIBRARIES      - shared library files
#
# User can set YAML_CPP_ROOT or CMAKE_SYSTEM_PREFIX_PATH
# to the preferred installation prefix

find_path(YAML_CPP_INCLUDE_DIRS yaml-cpp/yaml.h
    HINTS YAML_CPP_ROOT
    HINTS ENV YAML_CPP_ROOT
    HINTS CMAKE_SYSTEM_PREFIX_PATH
    PATH_SUFFIXES include
)

find_library(YAML_CPP_LIBRARIES
    NAMES yaml-cpp libyaml-cpp
    HINTS YAML_CPP_ROOT
    HINTS ENV YAML_CPP_ROOT
    HINTS CMAKE_SYSTEM_PREFIX_PATH
    PATH_SUFFIXES lib
)

mark_as_advanced(YAML_CPP_INCLUDE_DIRS YAML_CPP_LIBRARIES)