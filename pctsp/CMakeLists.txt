add_library(pypctsp SHARED "pypctsp.cpp")
set_target_properties(pypctsp PROPERTIES SUFFIX ${PYTHON_MODULE_EXTENSION})
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 17)

# setting for Mac OS X
if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set_target_properties( pypctsp PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
endif()

target_include_directories(pypctsp PUBLIC ${PCTSP_INCLUDE_DIRS})
target_include_directories(pypctsp PRIVATE
	${Boost_INCLUDE_DIRS}
	${Python3_INCLUDE_DIRS}
	${SCIP_INCLUDE_DIRS}
	${pybind11_INCLUDE_DIRS}
)
target_link_libraries(pypctsp PRIVATE
	${Boost_LIBRARIES}
	${SCIP_LIBRARIES}
	pybind11::headers
	pctsp
)

install (TARGETS pypctsp LIBRARY DESTINATION "pctsp")
