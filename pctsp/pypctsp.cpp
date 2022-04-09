
#include "pctsp/algorithms.hh"
#include "pctsp/heuristic.hh"
#include "pctsp/renaming.hh"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

namespace py = pybind11; 

typedef boost::bimap<PCTSPvertex, PCTSPvertex> VertexBimap;

// algorithms binding

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> pyBasicSolvePrizeCollectingTSP(
    py::object& model,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& heuristic_edges,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name
) {
    PyObject* capsule = model.attr("to_ptr")(false).ptr();
    SCIP* scip = (SCIP*) PyCapsule_GetPointer(capsule, "scip");
    PCTSPgraph graph;
    auto sol_edges = solvePrizeCollectingTSP(scip, graph, edge_list, heuristic_edges, cost_dict, prize_dict, quota, root_vertex, name);
    return sol_edges;
}

/** Call the solving method from python with SCIP */
std::vector<std::pair<PCTSPvertex, PCTSPvertex>> pySolvePrizeCollectingTSP(
    py::object& model,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& heuristic_edges,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    int branching_max_depth,
    unsigned int branching_strategy,
    bool cost_cover_disjoint_paths,
    bool cost_cover_shortest_path,
    bool cycle_cover,
    std::map<PCTSPvertex, CostNumberType>& disjoint_paths_map,
    int log_level_py,
    std::string& name,
    bool sec_disjoint_tour,
    double sec_lp_gap_improvement_threshold,
    bool sec_maxflow_mincut,
    int sec_max_tailing_off_iterations,
    int sec_sepafreq,
    std::filesystem::path solver_dir,
    float time_limit
) {
    PCTSPinitLogging(getBoostLevelFromPyLevel(log_level_py));

    // get the model from python
    PyObject* capsule = model.attr("to_ptr")(false).ptr();
    SCIP* scip = (SCIP*) PyCapsule_GetPointer(capsule, "scip");

    // rename vertices if they are not in range [0, n-1]
    PCTSPgraph graph;
    VertexBimap vertex_bimap;
    auto new_edges = renameEdges(vertex_bimap, edge_list);
    addEdgesToGraph(graph, new_edges);

    // rename the edges in the heuristic solution and the root vertex
    auto heur_edges_renamed = getNewEdges(vertex_bimap, heuristic_edges);
    auto heur_edges = edgesFromVertexPairs(graph, heur_edges_renamed);
    auto new_root = getNewVertex(vertex_bimap, root_vertex);

    // fill the cost map and prize map using renamed vertices
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    std::vector<PrizeNumberType> disjoint_paths_costs (boost::num_vertices(graph));
    fillCostMapFromRenamedMap(graph, cost_map, cost_dict, vertex_bimap);
    fillRenamedVertexMap(prize_map, prize_dict, vertex_bimap);
    fillRenamedVertexMap(disjoint_paths_costs, disjoint_paths_map, vertex_bimap);

    // call the branch and cut algorithm
    auto solution_edges = solvePrizeCollectingTSP(
        scip,
        graph,
        heur_edges,
        cost_map,
        prize_map,
        quota,
        new_root,
        branching_max_depth,
        branching_strategy,
        cost_cover_disjoint_paths,
        cost_cover_shortest_path,
        cycle_cover,
        disjoint_paths_costs,
        name,
        sec_disjoint_tour,
        sec_lp_gap_improvement_threshold,
        sec_maxflow_mincut,
        sec_max_tailing_off_iterations,
        sec_sepafreq,
        solver_dir,
        time_limit
    );
    // give old names to vertices in returned edges
    return getOldEdges(vertex_bimap, solution_edges);
}

/** Example of creating a pyscipopt model in CPP then exposing it to python */
py::object modelFromCpp() {
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    PyObject * capsule = PyCapsule_New((void*)scip, "scip", NULL);
    auto pyscipopt = py::module::import("pyscipopt.scip");
    auto Model = pyscipopt.attr("Model");
    auto from_ptr = Model.attr("from_ptr");
    auto take_ownership = py::bool_(true);
    py::object capsule_obj = py::reinterpret_borrow<py::object>(capsule);
    py::object model = from_ptr(capsule_obj, take_ownership);
    return model;
}

// heuristic bindings

std::vector<PCTSPvertex> collapseBind(
    std::list<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::list<PCTSPvertex>& py_tour,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PrizeNumberType& quota,
    PCTSPvertex& py_root,
    bool collapse_shortest_paths = false,
    int log_level_py = PyLoggingLevels::WARNING
) {
    PCTSPinitLogging(getBoostLevelFromPyLevel(log_level_py));

    // get renamed graph
    PCTSPgraph graph;
    VertexBimap vertex_bimap;
    auto new_edges = renameEdges(vertex_bimap, edge_list);
    addEdgesToGraph(graph, new_edges);
    auto root_vertex = getNewVertex(vertex_bimap, py_root);
    auto new_vertices = getNewVertices(vertex_bimap, py_tour);
    std::list<PCTSPvertex> tour (new_vertices.begin(), new_vertices.end());
 
    // fill the cost map and prize map using renamed vertices
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillCostMapFromRenamedMap(graph, cost_map, cost_dict, vertex_bimap);
    fillRenamedVertexMap(prize_map, prize_dict, vertex_bimap);

    // run the collapse algorithm and return renamed vertices
    auto new_tour = collapse(graph, tour, cost_map, prize_map, quota, root_vertex, collapse_shortest_paths);
    return getOldVertices(vertex_bimap, new_tour);
}

std::vector<PCTSPvertex> extensionBind(
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::list<PCTSPvertex>& py_tour,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PCTSPvertex& py_root,
    int step_size,
    int path_depth_limit,
    int log_level_py = PyLoggingLevels::WARNING
) {
    PCTSPinitLogging(getBoostLevelFromPyLevel(log_level_py));

    // get renamed graph
    PCTSPgraph graph;
    VertexBimap vertex_bimap;
    auto new_edges = renameEdges(vertex_bimap, edge_list);
    addEdgesToGraph(graph, new_edges);
    auto root_vertex = getNewVertex(vertex_bimap, py_root);
    auto new_vertices = getNewVertices(vertex_bimap, py_tour);
    std::list<PCTSPvertex> tour (new_vertices.begin(), new_vertices.end());

    // fill the cost map and prize map using renamed vertices
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillCostMapFromRenamedMap(graph, cost_map, cost_dict, vertex_bimap);
    fillRenamedVertexMap(prize_map, prize_dict, vertex_bimap);

    // run the extension algorithm
    extension(graph, tour, cost_map, prize_map, root_vertex, step_size, path_depth_limit);
    return getOldVertices(vertex_bimap, tour);
}

std::vector<PCTSPvertex> extensionUntilPrizeFeasibleBind(
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::list<PCTSPvertex>& py_tour,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PCTSPvertex& py_root,
    PrizeNumberType& quota,
    int step_size,
    int path_depth_limit,
    int log_level_py = PyLoggingLevels::WARNING
) {
    PCTSPinitLogging(getBoostLevelFromPyLevel(log_level_py));

    // get renamed graph
    PCTSPgraph graph;
    VertexBimap vertex_bimap;
    auto new_edges = renameEdges(vertex_bimap, edge_list);
    addEdgesToGraph(graph, new_edges);
    auto root_vertex = getNewVertex(vertex_bimap, py_root);
    auto new_vertices = getNewVertices(vertex_bimap, py_tour);
    std::list<PCTSPvertex> tour (new_vertices.begin(), new_vertices.end());

    // fill the cost map and prize map using renamed vertices
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillCostMapFromRenamedMap(graph, cost_map, cost_dict, vertex_bimap);
    fillRenamedVertexMap(prize_map, prize_dict, vertex_bimap);

    // run the extension algorithm
    extensionUntilPrizeFeasible(graph, tour, cost_map, prize_map, root_vertex, quota, step_size, path_depth_limit);
    return getOldVertices(vertex_bimap, tour);
}

std::vector<PCTSPvertex> pathExtensionCollapseBind(
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::list<PCTSPvertex>& py_tour,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PCTSPvertex& py_root,
    PrizeNumberType& quota,
    bool collapse_shortest_paths = false,
    int step_size = 1
) {
    // get renamed graph
    PCTSPgraph graph;
    VertexBimap vertex_bimap;
    auto new_edges = renameEdges(vertex_bimap, edge_list);
    addEdgesToGraph(graph, new_edges);
    auto root_vertex = getNewVertex(vertex_bimap, py_root);
    auto new_vertices = getNewVertices(vertex_bimap, py_tour);
    std::list<PCTSPvertex> tour (new_vertices.begin(), new_vertices.end());

    // fill the cost map and prize map using renamed vertices
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillCostMapFromRenamedMap(graph, cost_map, cost_dict, vertex_bimap);
    fillRenamedVertexMap(prize_map, prize_dict, vertex_bimap);

    // run the extension algorithm
    auto new_tour = pathExtensionCollapse(graph, tour, cost_map, prize_map, quota, root_vertex, collapse_shortest_paths, step_size);
    return getOldVertices(vertex_bimap, new_tour);
}

PYBIND11_MODULE(libpypctsp, m) {
    // functions for branch and cut
    m.def("basic_solve_pctsp_bind", &pyBasicSolvePrizeCollectingTSP, "Solve PCTSP with basic branch and cut");
    m.def("solve_pctsp_bind", &pySolvePrizeCollectingTSP, "Solve PCTSP.");

    // functions for heuristics
    m.def("collapse_bind", &collapseBind, "Collapse heuristic bind.");
    m.def("extension_bind", &extensionBind, "Extension heuristic bind.");
    m.def("extension_until_prize_feasible_bind", &extensionUntilPrizeFeasibleBind, "Extension until prize feasible find.");
    m.def("path_extension_collapse_bind", &pathExtensionCollapseBind, "Path extension & collapse.");
    m.def("unitary_gain", &unitaryGain, "Calculate the unitary loss of a vertex.");
    m.def("unitary_loss", &unitaryLoss, "Calculate the unitary gain of a vertex.");
}