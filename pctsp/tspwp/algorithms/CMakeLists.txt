set(TARGET "pctsp")

Python3_add_library(${TARGET} SHARED "knapsack.cpp" "pctsp.cpp")

set_target_properties(${TARGET} PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})

# include header files
target_include_directories(${TARGET} PUBLIC ${PCTSP_INCLUDE_DIRS})

# include and link to external dependencies, e.g. boost, scip
target_include_directories(${TARGET} PRIVATE
	${Python3_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${SCIP_INCLUDE_DIRS}
)
target_link_libraries(${TARGET} PUBLIC
	${Boost_LIBRARIES}
	${Python3_LIBRARIES}
	${SCIP_LIBRARIES}
	"preprocessing"
)
# target_link_libraries(${TARGET} )

# compiler options
target_compile_options(${TARGET} PRIVATE -std=c++17)    # prevents graph-tool warnings

get_filename_component(CURRENT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
if(${SRC_DIR} STREQUAL ${CURRENT_SRC_DIR}) 
    set(DST_DIR ".") # if parent is the base folder
else()
	file(RELATIVE_PATH DST_DIR ${SRC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})  
endif()

# install c++ library
install(TARGETS ${TARGET} DESTINATION ${DST_DIR} COMPONENT "EXT")

# move python files
file(GLOB PYFILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.py")
install(FILES ${PYFILES} DESTINATION ${DST_DIR} COMPONENT "PY")

set(TARGET_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/lib${TARGET})
message("${TARGET} LIBS: " ${TARGET_LIBRARIES})
list(APPEND PCTSP_LIBRARIES ${TARGET_LIBRARIES})