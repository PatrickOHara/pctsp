
set(TARGET "pctsp")
add_library(${TARGET} SHARED "algorithms.cpp" "graph.cpp" "heuristic.cpp" "knapsack.cpp" "preprocessing.cpp")
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

# set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER "algorithms.hh constraint.hh exception.hh graph.hh heuristic.hh knapsack.hh preprocessing.hh pyutils.hh subtour_elimination.hh valid_inequality.hh walk.hh")
set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/algorithms.hh
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/constraint.hh 
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/exception.hh
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/graph.hh
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/heuristic.hh
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/knapsack.hh
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/preprocessing.hh
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/pyutils.hh
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/subtour_elimination.hh
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/valid_inequality.hh
    ${PCTSP_INCLUDE_DIRS}/${TARGET}/walk.hh
)

if (DEFINED ENV{CMAKE_INSTALL_PREFIX})
    set(PCTSP_LIB_INSTALL_PREFIX $ENV{CMAKE_INSTALL_PREFIX})
else ()
    set(PCTSP_LIB_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()
message("PCTSP_LIB_INSTALL_PREFIX: " ${PCTSP_LIB_INSTALL_PREFIX})
install(TARGETS ${TARGET}
    LIBRARY DESTINATION ${PCTSP_LIB_INSTALL_PREFIX}/lib
    PUBLIC_HEADER DESTINATION ${PCTSP_LIB_INSTALL_PREFIX}/include/${TARGET})
