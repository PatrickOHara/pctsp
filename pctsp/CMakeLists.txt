Python3_add_library(pctsp_graph SHARED "graph.cpp")
set_target_properties(pctsp_graph PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
target_compile_options(pctsp_graph PRIVATE -std=c++17)
target_include_directories(pctsp_graph PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(pctsp_graph PRIVATE
	${Python3_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(pctsp_graph PRIVATE
	${Boost_LIBRARIES}
	${Python3_LIBRARIES}
)

Python3_add_library(knapsack SHARED "knapsack.cpp")
set_target_properties(knapsack PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
target_compile_options(knapsack PRIVATE -std=c++17)
target_include_directories(knapsack PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(knapsack PRIVATE ${SCIP_INCLUDE_DIRS})
target_link_libraries(knapsack PRIVATE ${SCIP_LIBRARIES})

Python3_add_library(preprocessing SHARED "preprocessing.cpp")
set_target_properties(preprocessing PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
target_compile_options(preprocessing PRIVATE -std=c++17)
target_include_directories(preprocessing PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(preprocessing PRIVATE
	${Python3_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(preprocessing PRIVATE
	${Boost_LIBRARIES}
	${Python3_LIBRARIES}
    pctsp_graph
)

Python3_add_library(algorithms SHARED "algorithms.cpp")
set_target_properties(algorithms PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
target_compile_options(algorithms PRIVATE -std=c++17)
target_include_directories(algorithms PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(algorithms PRIVATE
	${Python3_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
    ${SCIP_INCLUDE_DIRS}
)
target_link_libraries(algorithms PRIVATE
	${Boost_LIBRARIES}
	${Python3_LIBRARIES}
    ${SCIP_LIBRARIES}
    pctsp_graph
    preprocessing
)

set(PCTSP_LIBRARIES algorithms pctsp_graph knapsack preprocessing)



# Python3_add_library(preprocessing SHARED "preprocessing.cpp")
# Python3_add_library(algorithms SHARED "algorithms.cpp")


# add modules - keep everything tidy in directories!
# add_subdirectory(algorithms)
# add_subdirectory(constants)
# add_subdirectory(heuristic)
# add_subdirectory(preprocessing)


# get_filename_component(CURRENT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
# if(${SRC_DIR} STREQUAL ${CURRENT_SRC_DIR}) 
#     set(DST_DIR ".") # if parent is the base folder
# else()
# 	file(RELATIVE_PATH DST_DIR ${SRC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})  
# endif()

# # install c++ library
# install(TARGETS ${TARGET} DESTINATION ${DST_DIR} COMPONENT "EXT")

# copy all the py modules to dist folder
# file(RELATIVE_PATH DST_DIR ${SRC_DIR} "${CMAKE_CURRENT_SOURCE_DIR}")
# file(GLOB PYFILES LIST_DIRECTORIES false RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.py")
# install(FILES ${PYFILES} DESTINATION ${DST_DIR} COMPONENT "PY") # copy all the py-files in dir