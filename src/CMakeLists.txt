
set(TARGET "pctsp")
add_library(${TARGET} SHARED "algorithms.cpp" "graph.cpp" "heuristic.cpp" "knapsack.cpp" "logger.cpp" "preprocessing.cpp")

# setting for Mac OS X - dynamically link python
if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
endif()

target_compile_options(${TARGET} PRIVATE -std=c++17)
target_include_directories(${TARGET} PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(${TARGET} PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
    ${SCIP_INCLUDE_DIRS}
)
target_link_libraries(${TARGET} PRIVATE
	${Boost_LIBRARIES}
    ${SCIP_LIBRARIES}
)

file(GLOB_RECURSE PCTSP_HEADER_FILES "${PCTSP_INCLUDE_DIRS}/${TARGET}/*.hh")
set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER "${PCTSP_HEADER_FILES}")

install(TARGETS ${TARGET}
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/${TARGET})
