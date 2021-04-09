#ifndef __PCTSP_PYUTILS__
#define __PCTSP_PYUTILS__

// useful functions for binding C++ to python

#include <boost/python.hpp>

namespace py = boost::python;

template <typename T>
inline std::list<T> pyListToStdList(const py::list &iterable) {
    return std::list<T>(py::stl_input_iterator<T>(iterable),
                        py::stl_input_iterator<T>());
}

template <class T> inline py::list stdListToPyList(std::list<T> std_list) {
    typename std::list<T>::iterator iter;
    boost::python::list list;
    for (iter = std_list.begin(); iter != std_list.end(); ++iter) {
        list.append(*iter);
    }
    return list;
}

template <class K, class V> py::dict stdMapToPyDict(std::map<K, V> map) {
    typename std::map<K, V>::iterator iter;
    py::dict dictionary;
    for (iter = map.begin(); iter != map.end(); ++iter) {
        dictionary[iter->first] = iter->second;
    }
    return dictionary;
}

template <class K, class V> std::map<K, V> pyDictToStdMap(py::dict dictionary) {
    std::map<K, V> map;
    py::list list = dictionary.items();

    py::ssize_t n = py::len(list);
    for (py::ssize_t i = 0; i < n; i++) {
        auto tuple = list[i];
        K key = py::extract<K>(tuple[0]);
        V value = py::extract<V>(tuple[1]);
        map[key] = value;
    }
    return map;
}

template <typename Graph, typename Edge>
py::list toPyListOfTuples(std::list<Edge> &edge_list) {

    std::list<py::tuple> list_of_tuples;
    // iterate over each edge and convert to a python tuple

    return stdListToPyList(list_of_tuples);
}
#endif