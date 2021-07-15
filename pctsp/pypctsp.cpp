
#include "pctsp/algorithms.hh"
#include "pctsp/heuristic.hh"
#include "pctsp/pygraph.hh"

// algorithms binding

py::list pctsp_branch_and_cut_bind(
    py::list& py_edge_list,
    py::dict& prize_dict,
    py::dict& cost_dict,
    int quota,
    int root_vertex,
    bool cost_cover_disjoint_paths,
    bool cost_cover_shortest_path,
    bool cost_cover_steiner_tree,
    py::str& log_filepath_py,
    int log_level_py,
    bool sec_disjoint_tour,
    int sec_disjoint_tour_freq,
    bool sec_maxflow_mincut,
    int sec_maxflow_mincut_freq,
    float time_limit
) {

    PCTSPinitLogging(getBoostLevelFromPyLevel(log_level_py));
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(py_edge_list, vertex_id_map);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    PCTSPvertex boost_root = getNewVertex(vertex_id_map, root_vertex);

    // add self loops to graph - we assume the input graph is simple
    if (hasSelfLoopsOnAllVertices(graph) == false) {
        addSelfLoopsToGraph(graph);
        assignZeroCostToSelfLoops(graph, cost_map);
    }
    // get the log file
    const char* log_filepath = NULL;
    bool print_scip = false;
    if (py::len(log_filepath_py) > 0) {
        print_scip = true;
        log_filepath = py::extract<char const*>(log_filepath_py);
    }
    // run branch and cut algorithm - returns a list of edges in solution
    std::vector<PCTSPedge> solution_edges;
    PCTSPbranchAndCut(
        graph,
        solution_edges,
        cost_map,
        prize_map,
        quota,
        boost_root,
        log_filepath,
        print_scip,
        sec_disjoint_tour,
        sec_disjoint_tour_freq,
        sec_maxflow_mincut,
        sec_maxflow_mincut_freq,
        time_limit
    );

    // convert list of edges to a python list of python tuples
    return getPyEdgeList(graph, vertex_id_map, solution_edges);
}

// graph bindings

bool graph_from_edge_list(py::list& edge_list, py::dict& prize_dict,
    py::dict& cost_dict) {
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    BOOST_ASSERT(boost::num_edges(graph) == py::len(edge_list));
    return true;
}

// heuristic bindings

py::list collapse_bind(py::list& edge_list, py::list& py_tour,
    py::dict& cost_dict, py::dict& prize_dict, int quota, int py_root, int log_level_py = PyLoggingLevels::INFO) {
    PCTSPinitLogging(getBoostLevelFromPyLevel(log_level_py));
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    auto root_vertex = getNewVertex(vertex_id_map, py_root);
    auto new_tour = collapse(graph, tour, cost_map, prize_map, quota, root_vertex);
    return getPyVertexList(vertex_id_map, new_tour);
}

py::list extend_bind(py::list& edge_list, py::list& py_tour,
    py::dict& cost_dict, py::dict& prize_dict) {
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    extend(graph, tour, cost_map, prize_map);
    return getPyVertexList(vertex_id_map, tour);
}

py::list extend_until_prize_feasible_bind(py::list& edge_list, py::list& py_tour, py::dict& cost_dict, py::dict& prize_dict, int quota) {
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    extendUntilPrizeFeasible(graph, tour, cost_map, prize_map, quota);
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