# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -D "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if (${Python_ROOT}=="")
  message(WARNING "Python_ROOT is not set. gtest may use the wrong python version")
endif()
# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                EXCLUDE_FROM_ALL)

enable_testing()

# name of module to test - should only need to change this
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false fixtures.cpp fixtures.hh test_*.cpp)


set(TARGET "test_pctsp")

# add any test files here
add_executable(${TARGET} ${TEST_SOURCES})

set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 17)

# find SCIP dependencies that need to be statically linked for testing:
find_package(Readline REQUIRED)
find_package(GMP REQUIRED)
find_package(ZLIB REQUIRED)
find_package(TBB REQUIRED)

# may be needed by SCIP on some Linux systems when statically linking for tests
# find_library(GFORTRAN_LIBRARIES NAMES gfortran)
# find_library(OPENBLAS_LIBRARIES NAMES libopenblas openblas libopenblas.so.0 libopenblas.0.dylib HINTS ${OPENBLAS_DIR} PATH_SUFFIXES lib)

# link libraries and include files
target_link_libraries(${TARGET} PRIVATE
    ${Python_LIBRARIES}        # link at compile time for building tests
    ${Boost_LIBRARIES}
    gtest
    gmock
    ${CMAKE_THREAD_LIBS_INIT}   # prevents -pthread issues
    ${SCIP_LIBRARIES}
    ${Readline_LIBRARY}
    ${GMP_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${YAML_CPP_LIBRARIES}
    ${TBB_LIBRARIES}
    # ${GFORTRAN_LIBRARIES}
    # ${OPENBLAS_LIBRARIES}
    pctsp
)
target_include_directories(${TARGET} PUBLIC
    ${PCTSP_INCLUDE_DIRS}
    ${Python_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${SCIP_INCLUDE_DIRS}
    ${Readline_INCLUDE_DIR}
    ${GMP_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${YAML_CPP_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
)
message("done testing")