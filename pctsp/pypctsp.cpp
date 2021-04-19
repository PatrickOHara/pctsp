
#include "pctsp/algorithms.hh"
#include "pctsp/graph.hh"
#include "pctsp/heuristic.hh"
#include "pctsp/pyutils.hh"

// algorithms binding

py::list pctsp_branch_and_cut_bind(py::list& py_edge_list, py::dict& prize_dict,
    py::dict& cost_dict, int quota,
    int py_root_vertex, py::str& py_log_filepath, int py_log_level = PyLoggingLevels::INFO) {

    PCTSPinitLogging(getBoostLevelFromPyLevel(py_log_level));
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
    // get the log file
    const char* log_filepath = NULL;
    if (py::len(py_log_filepath) > 0) {
        log_filepath = py::extract<char const*>(py_log_filepath);
    }
    // run branch and cut algorithm - returns a list of edges in solution
    std::list<PCTSPedge> edge_list;
    PCTSPbranchAndCut(graph, edge_list, cost_map, prize_map, quota,
        root_vertex, log_filepath);

    // convert list of edges to a python list of python tuples
    return getPyEdgeList(graph, vertex_id_map, edge_list);
}

// graph bindings

bool graph_from_edge_list(py::list& edge_list, py::dict& prize_dict,
    py::dict& cost_dict) {
    VertexIdMap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    BOOST_ASSERT(boost::num_edges(graph) == py::len(edge_list));
    return true;
}

// heuristic bindings

py::list collapse_bind(py::list& edge_list, py::list& py_tour,
    py::dict& cost_dict, py::dict& prize_dict, int quota, int py_root, int py_log_level = PyLoggingLevels::INFO) {
    PCTSPinitLogging(getBoostLevelFromPyLevel(py_log_level));
    VertexIdMap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    int root_vertex = getBoostVertex(vertex_id_map, py_root);
    auto new_tour = collapse(graph, tour, cost_map, prize_map, quota, root_vertex);
    return getPyVertexList(vertex_id_map, new_tour);
}

py::list extend_bind(py::list& edge_list, py::list& py_tour,
    py::dict& cost_dict, py::dict& prize_dict) {
    VertexIdMap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    extend(graph, tour, cost_map, prize_map);
    return getPyVertexList(vertex_id_map, tour);
}

py::list extend_until_prize_feasible_bind(py::list& edge_list, py::list& py_tour, py::dict& cost_dict, py::dict& prize_dict, int quota) {
    VertexIdMap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    extend_until_prize_feasible(graph, tour, cost_map, prize_map, quota);
    return getPyVertexList(vertex_id_map, tour);
}

BOOST_PYTHON_MODULE(libpypctsp) {
    using namespace py;
    Py_Initialize();

    // graph
    def("graph_from_edge_list", graph_from_edge_list);

    // heuristics
    def("collapse_bind", collapse_bind);
    def("extend_bind", extend_bind);
    def("extend_until_prize_feasible_bind", extend_until_prize_feasible_bind);
    def("unitary_gain", unitary_gain);


    // algorithms
    def("pctsp_branch_and_cut_bind", pctsp_branch_and_cut_bind);
}