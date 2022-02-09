#ifndef __PCTSP_PYUTILS__
#define __PCTSP_PYUTILS__

// useful functions for binding C++ to python

#define BOOST_PYTHON_MAX_ARITY 30   // max num args in python function

#include <boost/python.hpp>

namespace py = boost::python;


template< typename Vertex>
std::list<std::pair<Vertex, Vertex>> toStdListOfPairs(boost::python::list& py_list) {
    int list_size = boost::python::len(py_list);
    std::list < std::pair<Vertex, Vertex> >std_list;
    for (int i = 0; i < list_size; i++) {
        boost::python::tuple py_tuple = boost::python::extract<boost::python::tuple>(py_list[i]);
        Vertex first = boost::python::extract<Vertex>(py_tuple[0]);
        Vertex second = boost::python::extract<Vertex>(py_tuple[1]);
        std::pair<Vertex, Vertex> std_pair(first, second);
        std_list.push_back(std_pair);
    }
    return std_list;
}
#endif