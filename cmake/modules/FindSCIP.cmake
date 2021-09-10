# - Try to find SCIP
# Once done this will define
#  SCIP_FOUND        - System has SCIP
#  SCIP_INCLUDE_DIRS - The SCIP include directories
# User can set SCIP_ROOT to the preferred installation prefix

find_path(SCIP_INCLUDE scip/scip.h
          HINTS SCIP_ROOT
          HINTS ENV SCIP_ROOT
          HINTS ENV CMAKE_SYSTEM_PREFIX_PATH
          PATH_SUFFIXES include)

find_library(SCIP_LIBRARIES
          NAMES libscip scip
          HINTS SCIP_ROOT
          HINTS ENV SCIP_ROOT
          HINTS ENV CMAKE_SYSTEM_PREFIX_PATH
          PATH_SUFFIXES lib
)

find_file(SCIP_CONFIG_LOC scip/config.h
  HINTS ${SCIP_INCLUDE}
  PATH_SUFFIXES include)

if(NOT "${SCIP_CONFIG_LOC}" STREQUAL "SCIP_CONFIG_LOC-NOTFOUND")
  file(READ "${SCIP_CONFIG_LOC}" SCIP_CONFIG)
  string(REGEX MATCH "\#define SCIP_VERSION_MAJOR +([0-9]+)" _ "${SCIP_CONFIG}")
  set(SCIP_VERSION_MAJOR "${CMAKE_MATCH_1}")
  string(REGEX MATCH "\#define SCIP_VERSION_MINOR +([0-9]+)" _ "${SCIP_CONFIG}")
  set(SCIP_VERSION_MINOR "${CMAKE_MATCH_1}")
  string(REGEX MATCH "\#define SCIP_VERSION_PATCH +([0-9]+)" _ "${SCIP_CONFIG}")
  set(SCIP_VERSION_PATCH "${CMAKE_MATCH_1}")
  set(SCIP_VERSION "${SCIP_VERSION_MAJOR}.${SCIP_VERSION_MINOR}.${SCIP_VERSION_PATCH}")
  unset(SCIP_CONFIG)
endif()
unset(SCIP_CONFIG_LOC)

# handle the QUIETLY and REQUIRED arguments and set SCIP_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SCIP
  FOUND_VAR SCIP_FOUND
  REQUIRED_VARS SCIP_INCLUDE SCIP_LIBRARIES
  VERSION_VAR SCIP_VERSION
  FAIL_MESSAGE "Could NOT find SCIP, use SCIP_ROOT to hint its location"
)

mark_as_advanced(SCIP_INCLUDE SCIP_CONFIG_LOC)

set(SCIP_INCLUDE_DIRS ${SCIP_INCLUDE})