add_library(pypctsp SHARED "pypctsp.cpp")
set_target_properties(pypctsp PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
target_compile_options(pypctsp PRIVATE -std=c++17)

# setting for Mac OS X
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

install (TARGETS pypctsp LIBRARY DESTINATION "pctsp")
