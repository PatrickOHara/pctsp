# Try to find SCIPSDP
# User can set SCIPSDP_ROOT to the preferred installation prefix

message("SCIPSDP_ROOT: " ${SCIPSDP_ROOT})
find_path(SCIPSDP_INCLUDE scip/scipsdpdefplugins.h HINTS SCIPSDP_ROOT SCIP_ROOT PATH_SUFFIXES include)
find_library(SCIPSDP_LIBRARIES NAMES scipsdp HINTS SCIPSDP_ROOT SCIP_ROOT PATH_SUFFIXES lib)
find_package_handle_standard_args(SCIPSDP
    FOUND_VAR SCIPSDP_FOUND
    REQUIRED_VARS SCIPSDP_INCLUDE SCIPSDP_LIBRARIES
    FAIL_MESSAGE "Could NOT find SCIPSDP. Use SCIPSDP_ROOT to hint at its location."
)