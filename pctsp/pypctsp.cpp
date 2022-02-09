
#include "pctsp/algorithms.hh"
#include "pctsp/heuristic.hh"
#include "pctsp/pygraph.hh"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// algorithms binding

// pybind11::object model_pctsp_bind(
//     // pybind11::capsule& py_scip,
//     // PyObject* capsule,
//     std::string& name,
//     pybind11::list& py_edge_list,
//     pybind11::dict& prize_dict,
//     pybind11::dict& cost_dict,
//     PrizeNumberType& quota,
//     int& root_vertex,
//     pybind11::list& initial_solution_py
// ) {

pybind11::object model_pctsp_bind(
    std::string& name,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& solution_edges
) {
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    modelPrizeCollectingTSP(scip, edge_list, solution_edges, prize_dict, cost_dict, quota, root_vertex);
    PyObject * capsule = PyCapsule_New((void*)scip, name.c_str(), NULL);
    auto pyscipopt = pybind11::module::import("pyscipopt.scip");
    auto Model = pyscipopt.attr("Model");
    auto from_ptr = Model.attr("from_ptr");
    auto take_ownership = pybind11::bool_(true);
    pybind11::object capsule_obj = pybind11::reinterpret_borrow<pybind11::object>(capsule);
    pybind11::object model = from_ptr(capsule_obj, take_ownership);
    return model;
}

py::list pctsp_branch_and_cut_bind(
    py::list& py_edge_list,
    py::dict& prize_dict,
    py::dict& cost_dict,
    int quota,
    int root_vertex,
    py::str& bounds_csv_filepath_py,
    bool cost_cover_disjoint_paths,
    bool cost_cover_shortest_path,
    bool cost_cover_steiner_tree,
    bool cycle_cover,
    py::dict& disjoint_paths_cost,
    py::list& initial_solution_py,
    py::str& log_boost_filepath_py,
    py::str& log_scip_filepath_py,
    int logging_level_py,
    py::str& metrics_csv_filepath_py,
    py::str& name_py,
    bool sec_disjoint_tour,
    int sec_disjoint_tour_freq,
    bool sec_maxflow_mincut,
    int sec_maxflow_mincut_freq,
    py::str& summary_yaml_filepath_py,
    float time_limit
) {

    PCTSPinitLogging(getBoostLevelFromPyLevel(logging_level_py));
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(py_edge_list, vertex_id_map);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillPrizeMapFromPyDict(prize_map, prize_dict, vertex_id_map);
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    fillCostMapFromPyDict(graph, cost_map, cost_dict, vertex_id_map);
    PCTSPvertex boost_root = getNewVertex(vertex_id_map, root_vertex);
    std::vector<int> disjoint_paths_distances;
    if (cost_cover_disjoint_paths)
        disjoint_paths_distances = getVertexPropertyVectorFromPyDict(disjoint_paths_cost, graph, vertex_id_map);

    std::vector<PCTSPedge> solution_edges;
    if (py::len(initial_solution_py) > 0) {
        std::list<std::pair<int, int>> initial_solution_std = toStdListOfPairs<int>(initial_solution_py);
        auto initial_solution_pairs = getNewEdges(vertex_id_map, initial_solution_std);
        auto pairs_first = initial_solution_pairs.begin();
        auto pairs_last = initial_solution_pairs.end();
        solution_edges = edgesFromVertexPairs(graph, pairs_first, pairs_last);
    }

    // add self loops to graph - we assume the input graph is simple
    if (hasSelfLoopsOnAllVertices(graph) == false) {
        addSelfLoopsToGraph(graph);
        assignZeroCostToSelfLoops(graph, cost_map);
    }
    std::string name = py::extract<std::string>(name_py);

    // get the log file
    std::string bounds_csv_filepath = py::extract<std::string>(bounds_csv_filepath_py);
    std::string log_scip_filepath = py::extract<std::string>(log_scip_filepath_py);
    std::string metrics_csv_filepath = py::extract<std::string>(metrics_csv_filepath_py);
    std::string summary_yaml_filepath = py::extract<std::string>(summary_yaml_filepath_py);
    // run branch and cut algorithm - returns a list of edges in solution
    PCTSPbranchAndCut(
        graph,
        solution_edges,
        cost_map,
        prize_map,
        quota,
        boost_root,
        bounds_csv_filepath,
        cost_cover_disjoint_paths,
        cost_cover_shortest_path,
        cost_cover_steiner_tree,
        cycle_cover,
        disjoint_paths_distances,
        log_scip_filepath,
        metrics_csv_filepath,
        name,
        sec_disjoint_tour,
        sec_disjoint_tour_freq,
        sec_maxflow_mincut,
        sec_maxflow_mincut_freq,
        summary_yaml_filepath,
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
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillPrizeMapFromPyDict(prize_map, prize_dict, vertex_id_map);
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    fillCostMapFromPyDict(graph, cost_map, cost_dict, vertex_id_map);
    BOOST_ASSERT(boost::num_edges(graph) == py::len(edge_list));
    return true;
}

// heuristic bindings

py::list collapse_bind(
    py::list& edge_list,
    py::list& py_tour,
    py::dict& cost_dict,
    py::dict& prize_dict,
    int quota,
    int py_root,
    bool collapse_shortest_paths = false,
    int log_level_py = PyLoggingLevels::INFO
) {
    PCTSPinitLogging(getBoostLevelFromPyLevel(log_level_py));
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillPrizeMapFromPyDict(prize_map, prize_dict, vertex_id_map);
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    fillCostMapFromPyDict(graph, cost_map, cost_dict, vertex_id_map);
    auto root_vertex = getNewVertex(vertex_id_map, py_root);
    auto new_tour = collapse(graph, tour, cost_map, prize_map, quota, root_vertex, collapse_shortest_paths);
    return getPyVertexList(vertex_id_map, new_tour);
}

py::list extend_bind(py::list& edge_list, py::list& py_tour,
    py::dict& cost_dict, py::dict& prize_dict) {
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillPrizeMapFromPyDict(prize_map, prize_dict, vertex_id_map);
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    fillCostMapFromPyDict(graph, cost_map, cost_dict, vertex_id_map);
    extend(graph, tour, cost_map, prize_map);
    return getPyVertexList(vertex_id_map, tour);
}

py::list extension_bind(
    py::list& edge_list,
    py::list& py_tour,
    py::dict& cost_dict,
    py::dict& prize_dict,
    int py_root,
    int step_size,
    int path_depth_limit
) {
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillPrizeMapFromPyDict(prize_map, prize_dict, vertex_id_map);
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    fillCostMapFromPyDict(graph, cost_map, cost_dict, vertex_id_map);
    auto root_vertex = getNewVertex(vertex_id_map, py_root);
    extension(graph, tour, cost_map, prize_map, root_vertex, step_size, path_depth_limit);
    return getPyVertexList(vertex_id_map, tour);
}

py::list extension_until_prize_feasible_bind(
    py::list& edge_list,
    py::list& py_tour,
    py::dict& cost_dict,
    py::dict& prize_dict,
    int py_root,
    int quota,
    int step_size,
    int path_depth_limit
) {
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillPrizeMapFromPyDict(prize_map, prize_dict, vertex_id_map);
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    fillCostMapFromPyDict(graph, cost_map, cost_dict, vertex_id_map);
    auto root_vertex = getNewVertex(vertex_id_map, py_root);
    extensionUntilPrizeFeasible(graph, tour, cost_map, prize_map, root_vertex, quota, step_size, path_depth_limit);
    return getPyVertexList(vertex_id_map, tour);
}

py::list extend_until_prize_feasible_bind(py::list& edge_list, py::list& py_tour, py::dict& cost_dict, py::dict& prize_dict, int quota) {
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    auto tour = getBoostVertexList(vertex_id_map, py_tour);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillPrizeMapFromPyDict(prize_map, prize_dict, vertex_id_map);
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    fillCostMapFromPyDict(graph, cost_map, cost_dict, vertex_id_map);
    extendUntilPrizeFeasible(graph, tour, cost_map, prize_map, quota);
    return getPyVertexList(vertex_id_map, tour);
}

// BOOST_PYTHON_MODULE(libpypctsp) {
//     using namespace py;
//     Py_Initialize();

//     // graph
//     def("graph_from_edge_list", graph_from_edge_list);

//     // heuristics
//     def("collapse_bind", collapse_bind);
//     def("extend_bind", extend_bind);
//     def("extend_until_prize_feasible_bind", extend_until_prize_feasible_bind);
//     def("extension_bind", extension_bind);
//     def("extension_until_prize_feasible_bind", extension_until_prize_feasible_bind);
//     def("model_pctsp_bind", model_pctsp_bind);
//     def("unitary_gain", unitary_gain);

//     // algorithms
//     def("pctsp_branch_and_cut_bind", pctsp_branch_and_cut_bind);
// }

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(libpypctsp, m) {
    m.def("add", &add, "A function that adds two numbers");
    m.def("model_pctsp_bind", &model_pctsp_bind, "Model PCTSP");
}