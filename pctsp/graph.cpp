#include "pctsp/graph.hh"
#include "pctsp/exception.hh"
#include <iostream>

using namespace std;

int findOrInsertVertex(PCTSPgraph &graph, VertexIdMap &vertex_id_map,
                       int py_vertex, int &vertex_id) {
    int graph_vertex;
    auto it = vertex_id_map.right.find(py_vertex);
    if (it == vertex_id_map.right.end()) {
        vertex_id_map.insert(position(vertex_id, py_vertex));
        graph_vertex = vertex_id;
        boost::add_vertex(graph);
        vertex_id++;
    } else {
        graph_vertex = (*it).second;
    }
    return graph_vertex;
}

PCTSPgraph graphFromPyEdgeList(py::list &edge_list,
                               VertexIdMap &vertex_id_map) {
    PCTSPgraph graph;
    int vertex_id = 0;
    auto length = py::len(edge_list);
    for (int i = 0; i < length; i++) {
        py::tuple py_edge = py::extract<py::tuple>(edge_list[i]);

        int source = py::extract<int>(py_edge[0]);
        int target = py::extract<int>(py_edge[1]);

        int graph_source =
            findOrInsertVertex(graph, vertex_id_map, source, vertex_id);
        int graph_target =
            findOrInsertVertex(graph, vertex_id_map, target, vertex_id);
        boost::add_edge(graph_source, graph_target, graph);
    }

    return graph;
}

int getPyVertex(VertexIdMap &vertex_id_map, int vertex) {
    auto it = vertex_id_map.left.find(vertex);
    return (*it).second;
}
int getBoostVertex(VertexIdMap &vertex_id_map, int py_vertex) {
    auto it = vertex_id_map.right.find(py_vertex);
    return (*it).second;
}

PCTSPprizeMap prizeMapFromPyDict(py::dict &prize_dict,
                                 VertexIdMap &vertex_id_map) {
    PCTSPprizeMap prize_map;
    py::list prize_list = prize_dict.items();
    int n = py::len(prize_list);
    for (int i = 0; i < n; i++) {
        py::tuple prize_tuple = py::extract<py::tuple>(prize_list[i]);
        int py_vertex = py::extract<int>(prize_tuple[0]);
        int prize = py::extract<int>(prize_tuple[1]);
        int boost_vertex = getBoostVertex(vertex_id_map, py_vertex);
        prize_map[boost_vertex] = prize;
    }
    return prize_map;
}

PCTSPcostMap costMapFromPyDict(py::dict &cost_dict, PCTSPgraph &graph,
                               VertexIdMap &vertex_id_map) {
    PCTSPcostMap cost_map;
    py::list cost_list = cost_dict.items();
    int n = py::len(cost_list);
    for (int i = 0; i < n; i++) {
        py::tuple cost_tuple = py::extract<py::tuple>(cost_list[i]);
        py::tuple py_edge = py::extract<py::tuple>(cost_tuple[0]);
        int py_source = py::extract<int>(py_edge[0]);
        int py_target = py::extract<int>(py_edge[1]);
        int cost = py::extract<int>(cost_tuple[1]);

        int boost_source = getBoostVertex(vertex_id_map, py_source);
        int boost_target = getBoostVertex(vertex_id_map, py_target);
        auto boost_edge = boost::edge(boost_source, boost_target, graph);

        if (boost_edge.second == false) {
            throw EdgeNotFoundException(std::to_string(boost_source),
                                        std::to_string(boost_target));
        }
        cost_map[boost_edge.first] = cost;
    }
    return cost_map;
}

bool graph_from_edge_list(py::list &edge_list, py::dict &prize_dict,
                          py::dict &cost_dict) {
    VertexIdMap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    BOOST_ASSERT(boost::num_edges(graph) == py::len(edge_list));
    return true;
}

py::list getPyEdgeList(PCTSPgraph &graph, VertexIdMap &vertex_id_map,
                       std::list<PCTSPedge> &edge_list) {
    py::list py_list;
    for (auto it = edge_list.begin(); it != edge_list.end(); ++it) {
        PCTSPedge edge = *it;
        int source = boost::source(edge, graph);
        int target = boost::target(edge, graph);
        int py_source = getPyVertex(vertex_id_map, source);
        int py_target = getPyVertex(vertex_id_map, target);
        py::tuple py_edge = py::make_tuple(py_source, py_target);
        py_list.append(py_edge);
    }
    return py_list;
}

// void in_the_library(PyObject *obj)
// {
//     std::cout << "  PyList_Check(): " << PyList_Check(obj) << std::endl <<
//     std::flush; std::cout << "Our &PyDict_Type: " << &PyList_Type <<
//     std::endl << std::flush; std::cout << "        Its type: " <<
//     obj->ob_type << std::endl << std::flush; py::list d =
//     py::extract<py::list>(obj)(); // <= exception potentially raised here
// }

// void graph_from_edge_list(py::list &py_list) {
//     std::cout << "Hello, world!" << endl;
//     // in_the_library(&py_list);
//     // py::list my_list = py::extract<py::list>(py_list);
//     std::cout << "length: " << py::len(py_list) << endl;
// }

BOOST_PYTHON_MODULE(libgraph) {
    using namespace py;
    Py_Initialize();
    def("graph_from_edge_list", graph_from_edge_list);
}