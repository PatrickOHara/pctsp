
# add the graph library
add_library(graph SHARED "graph.cpp")
set_target_properties(graph PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
target_compile_options(graph PRIVATE -std=c++17)
if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set_target_properties( graph PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
endif()
target_include_directories(graph PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(graph PRIVATE
	${Python3_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(graph PRIVATE
	${Boost_LIBRARIES}
)


# add the knapsack library
add_library(knapsack SHARED "knapsack.cpp")
set_target_properties(knapsack PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
target_compile_options(knapsack PRIVATE -std=c++17)
target_include_directories(knapsack PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(knapsack PRIVATE ${SCIP_INCLUDE_DIRS})
target_link_libraries(knapsack PRIVATE ${SCIP_LIBRARIES})

# add the preprocessing library
add_library(preprocessing SHARED "preprocessing.cpp")
set_target_properties(preprocessing PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
target_compile_options(preprocessing PRIVATE -std=c++17)
if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set_target_properties( preprocessing PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
endif()
target_include_directories(preprocessing PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(preprocessing PRIVATE
	${Python3_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(preprocessing PRIVATE
	${Boost_LIBRARIES}
    graph
)

add_library(algorithms SHARED "algorithms.cpp")
set_target_properties(algorithms PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set_target_properties( algorithms PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
endif()
target_compile_options(algorithms PRIVATE -std=c++17)
target_include_directories(algorithms PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(algorithms PRIVATE
	${Python3_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
    ${SCIP_INCLUDE_DIRS}
)
target_link_libraries(algorithms PRIVATE
	${Boost_LIBRARIES}
    ${SCIP_LIBRARIES}
    graph
    preprocessing
)

set(PCTSP_LIBRARIES algorithms graph knapsack preprocessing)
