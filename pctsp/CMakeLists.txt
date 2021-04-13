add_library(pypctsp SHARED "pypctsp.cpp")
set_target_properties(pypctsp PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
target_compile_options(pypctsp PRIVATE -std=c++17)
if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set_target_properties( pypctsp PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
endif()
target_include_directories(pypctsp PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(pypctsp PRIVATE
	${Boost_INCLUDE_DIRS}
	${Python3_INCLUDE_DIRS}
	${SCIP_INCLUDE_DIRS}
)
target_link_libraries(pypctsp PRIVATE
	${Boost_LIBRARIES}
	${SCIP_LIBRARIES}
	pctsp
)
message("PYTHON_RELATIVE_SITE_PACKAGES_DIR: " ${PYTHON_RELATIVE_SITE_PACKAGES_DIR})
install(TARGETS pypctsp LIBRARY DESTINATION ${PYTHON_RELATIVE_SITE_PACKAGES_DIR}/pctsp)
# python_extension_module(graph)
# if(SKBUILD)
#   message(STATUS "The project is built using scikit-build")
# endif()

# add the knapsack library
# add_library(knapsack SHARED "knapsack.cpp")
# set_target_properties(knapsack PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
# target_compile_options(knapsack PRIVATE -std=c++17)
# target_include_directories(knapsack PUBLIC ${PCTSP_INCLUDE_DIRS})
# target_include_directories(knapsack PRIVATE ${SCIP_INCLUDE_DIRS})
# target_link_libraries(knapsack PRIVATE ${SCIP_LIBRARIES})

# # add the preprocessing library
# add_library(preprocessing SHARED "preprocessing.cpp")
# set_target_properties(preprocessing PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
# target_compile_options(preprocessing PRIVATE -std=c++17)
# if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
#     set_target_properties( preprocessing PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
# endif()
# target_include_directories(preprocessing PUBLIC ${PCTSP_INCLUDE_DIRS})
# target_include_directories(preprocessing PRIVATE
# 	${Python3_INCLUDE_DIRS}
# 	${Boost_INCLUDE_DIRS}
# )
# target_link_libraries(preprocessing PRIVATE
# 	${Boost_LIBRARIES}
#     graph
# )

# add_library(algorithms SHARED "algorithms.cpp")
# set_target_properties(algorithms PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
# if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
#     set_target_properties( algorithms PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
# endif()
# target_compile_options(algorithms PRIVATE -std=c++17)
# target_include_directories(algorithms PUBLIC ${PCTSP_INCLUDE_DIRS})
# target_include_directories(algorithms PRIVATE
# 	${Python3_INCLUDE_DIRS}
# 	${Boost_INCLUDE_DIRS}
#     ${SCIP_INCLUDE_DIRS}
# )
# target_link_libraries(algorithms PRIVATE
# 	${Boost_LIBRARIES}
#     ${SCIP_LIBRARIES}
#     graph
#     preprocessing
# )

# set(PCTSP_LIBRARIES algorithms graph knapsack preprocessing)
# install(TARGETS ${PCTSP_LIBRARIES} LIBRARY DESTINATION pctsp)
