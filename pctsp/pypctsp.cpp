
#include "pctsp/algorithms.hh"
#include "pctsp/graph.hh"


bool graph_from_edge_list(py::list &edge_list, py::dict &prize_dict,
                          py::dict &cost_dict) {
    VertexIdMap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    BOOST_ASSERT(boost::num_edges(graph) == py::len(edge_list));
    return true;
}

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

BOOST_PYTHON_MODULE(libpypctsp) {
    using namespace py;
    Py_Initialize();

    // graph
    def("graph_from_edge_list", graph_from_edge_list);

    // algorithms
    def("pctsp_branch_and_cut_bind", pctsp_branch_and_cut_bind);

}