/** Algorithms for the prize collecting TSP */

#include "pctsp/algorithms.hh"
#include <boost/python.hpp>
using namespace boost;
using namespace std;
namespace py = boost::python;

typedef int vertex_t;

py::list pctsp_branch_and_cut_bind(py::list &py_edge_list, py::dict &prize_dict,
                                   py::dict &cost_dict, int quota,
                                   int py_root_vertex) {

    VertexIdMap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(py_edge_list, vertex_id_map);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    int root_vertex = getBoostVertex(vertex_id_map, py_root_vertex);

    // add self loops to graph - we assume the input graph is simple
    if (hasSelfLoopsOnAllVertices(graph) == false) {
        addSelfLoopsToGraph(graph);
        assignZeroCostToSelfLoops(graph, cost_map);
    }
    // run branch and cut algorithm - returns a list of edges in solution
    std::list<PCTSPedge> edge_list;
    PCTSPbranchAndCut(graph, edge_list, cost_map, prize_map, quota,
                      root_vertex);

    // convert list of edges to a python list of python tuples
    return getPyEdgeList(graph, vertex_id_map, edge_list);
}

// void gt_test(gt::GraphInterface &gi, boost::any cost_map,
//              boost::any prize_map) {

//     typedef typename gt::vprop_map_t<int>::type vprop_t;
//     auto prize_int_property_unchecked =
//         boost::any_cast<vprop_t>(prize_map).get_unchecked();
//     typedef typename gt::eprop_map_t<int>::type eprop_t;
//     auto cost_int_property_unchecked =
//         boost::any_cast<eprop_t>(cost_map).get_unchecked();

//     auto gt_graph = gi.get_graph();
//     int n = boost::num_vertices(gt_graph);
//     PCTSPgraph graph;
//     PCTSPcostMap new_cost_map;
//     PCTSPprizeMap new_prize_map;
//     PCTSPgetAdjacencyGraph(gt_graph, prize_int_property_unchecked,
//                            cost_int_property_unchecked, graph, new_prize_map,
//                            new_cost_map);
//     BOOST_ASSERT(gi.get_num_vertices() == n);
//     BOOST_ASSERT(boost::num_vertices(graph) == n);
//     BOOST_ASSERT(boost::num_edges(graph) == n);

//     PCTSPgraph new_graph = addSelfLoopsToGraph(graph);
//     cout << "Test num vertices: ";
//     if (boost::num_vertices(new_graph) == boost::num_vertices(graph)) {
//         cout << "New and old graph have same number of vertices" << endl;
//     }
//     cout << "Test num edges: ";
//     if (boost::num_edges(new_graph) ==
//         boost::num_edges(graph) + boost::num_vertices(graph)) {
//         cout << "New graph has correct number of edges" << endl;
//     }
//     cout << "done" << endl;
// }

/**
py::dict test_py_dict_conversion(py::dict dict) {
    std::map<int, int> test_map = pyDictToStdMap<int, int>(dict);
    return stdMapToPyDict(test_map);
}

BOOST_PYTHON_MODULE(libpctsp) {
    using namespace py;
    def("pctsp_branch_and_cut_bind", pctsp_branch_and_cut_bind);
    def("test_py_dict_conversion", test_py_dict_conversion);
}
*/
BOOST_PYTHON_MODULE(libpctsp) {
    using namespace py;
    def("pctsp_branch_and_cut_bind", pctsp_branch_and_cut_bind);
}