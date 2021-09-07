
#include "pctsp/exception.hh"
#include "pctsp/pygraph.hh"

// graphs from python objects

PCTSPgraph graphFromPyEdgeList(py::list& edge_list, BoostPyBimap& lookup) {
    PCTSPgraph graph;
    auto old_edges = toStdListOfPairs<PCTSPvertex>(edge_list);
    auto new_edges = renameEdges(lookup, old_edges);
    auto start = new_edges.begin();
    auto end = new_edges.end();
    addEdgesToGraph(graph, start, end);
    return graph;
}

void fillPrizeMapFromPyDict(VertexPrizeMap& prize_map, py::dict& prize_dict,
    BoostPyBimap& vertex_id_map) {
    py::list prize_list = prize_dict.items();
    int n = py::len(prize_list);
    for (int i = 0; i < n; i++) {
        py::tuple prize_tuple = py::extract<py::tuple>(prize_list[i]);
        int py_vertex = py::extract<int>(prize_tuple[0]);
        int prize = py::extract<int>(prize_tuple[1]);
        int boost_vertex = getNewVertex(vertex_id_map, py_vertex);
        prize_map[boost_vertex] = prize;
    }
}

void fillCostMapFromPyDict(PCTSPgraph& graph, EdgeCostMap& cost_map, py::dict& cost_dict, BoostPyBimap& vertex_id_map) {
    py::list cost_list = cost_dict.items();
    int n = py::len(cost_list);
    for (int i = 0; i < n; i++) {
        py::tuple cost_tuple = py::extract<py::tuple>(cost_list[i]);
        py::tuple py_edge = py::extract<py::tuple>(cost_tuple[0]);
        int py_source = py::extract<int>(py_edge[0]);
        int py_target = py::extract<int>(py_edge[1]);
        int cost = py::extract<int>(cost_tuple[1]);

        auto boost_source = getNewVertex(vertex_id_map, py_source);
        auto boost_target = getNewVertex(vertex_id_map, py_target);
        auto boost_edge = boost::edge(boost_source, boost_target, graph);

        if (boost_edge.second == false) {
            throw EdgeNotFoundException(std::to_string(boost_source),
                std::to_string(boost_target));
        }
        cost_map[boost_edge.first] = cost;
    }
}

py::list getPyVertexList(BoostPyBimap& vertex_id_map, std::list<PCTSPvertex>& vertex_list) {
    py::list py_list;
    for (auto it = vertex_list.begin(); it != vertex_list.end(); it++) {
        auto vertex = *it;
        int py_vertex = getOldVertex(vertex_id_map, vertex);
        py_list.append(py_vertex);
    }
    return py_list;
}

std::list<PCTSPvertex> getBoostVertexList(BoostPyBimap& vertex_id_map, py::list& py_list) {
    std::list<PCTSPvertex> vertex_list;
    for (int i = 0; i < py::len(py_list); i++) {
        int py_vertex = py::extract<int>(py_list[i]);
        vertex_list.push_back(getNewVertex(vertex_id_map, py_vertex));
    }
    return vertex_list;
}