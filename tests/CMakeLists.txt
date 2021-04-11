# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
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

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                EXCLUDE_FROM_ALL)

enable_testing()

# name of module to test - should only need to change this
# set(TARGET_TO_TEST "heuristic" "pctsp" "preprocessing")
# file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false fixtures.cpp fixtures.hh test_*.cpp)
set(TARGET_TO_TEST knapsack graph preprocessing)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false "fixtures.cpp" "test_preprocessing.cpp" "test_knapsack.cpp" "test_graph.cpp" "test_pctsp.cpp")
set(TARGET "test_pctsp")

# add any test files here
add_executable(${TARGET} ${TEST_SOURCES})

target_compile_options(${TARGET} PRIVATE -std=c++17)

# link libraries and include files
target_link_libraries(${TARGET} PRIVATE
    ${Python3_LIBRARIES}
    ${Boost_LIBRARIES}
    gtest
    gmock
    ${TARGET_TO_TEST}
    ${CMAKE_THREAD_LIBS_INIT}   # prevents -pthread issues
    ${SCIP_LIBRARIES}
    ${PCTSP_LIBRARIES}
)
target_include_directories(${TARGET} PUBLIC
    ${PCTSP_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${SCIP_INCLUDE_DIRS}
)