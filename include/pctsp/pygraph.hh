#ifndef __PCTSP_PYPCTSP__
#define __PCTSP_PYPCTSP__

#include "graph.hh"
#include "pyutils.hh"
#include "renaming.hh"


// mapping from boost vertices to python vertices
typedef boost::bimap<PCTSPvertex, int> BoostPyBimap;

// functions for converting between python and boost

PCTSPgraph graphFromPyEdgeList(py::list& edge_list, BoostPyBimap& vertex_id_map);
void fillPrizeMapFromPyDict(VertexPrizeMap& prize_map, py::dict& prize_dict,
    BoostPyBimap& vertex_id_map);
void fillPrizeMapFromPyDict(VertexPrizeMap& prize_map, py::dict& prize_dict);
void fillCostMapFromPyDict(PCTSPgraph& graph, EdgeCostMap& cost_map, py::dict& cost_dict);
void fillCostMapFromPyDict(PCTSPgraph& graph, EdgeCostMap& cost_map, py::dict& cost_dict, BoostPyBimap& vertex_id_map);
py::list getPyVertexList(BoostPyBimap& vertex_id_map, std::list<PCTSPvertex>& vertex_list);
std::list<PCTSPvertex> getBoostVertexList(BoostPyBimap& vertex_id_map, py::list& py_list);

template <typename EdgeContainer>
py::list getPyEdgeList(PCTSPgraph& graph, BoostPyBimap& vertex_id_map,
    EdgeContainer& edge_list) {
    py::list py_list;
    for (auto it = edge_list.begin(); it != edge_list.end(); ++it) {
        PCTSPedge edge = *it;
        PCTSPvertex source = boost::source(edge, graph);
        PCTSPvertex target = boost::target(edge, graph);
        int py_source = getOldVertex(vertex_id_map, source);
        int py_target = getOldVertex(vertex_id_map, target);
        py::tuple py_edge = py::make_tuple(py_source, py_target);
        py_list.append(py_edge);
    }
    return py_list;
}

template<typename T, typename NewVertexIt, typename OldVertex, typename NewVertex>
std::vector<T> getVertexPropertyVectorFromPyDict(
    py::dict& py_property_map,
    NewVertexIt& first,
    NewVertexIt& last,
    boost::bimap<NewVertex, OldVertex>& vertex_id_map
) {
    auto num_vertices = std::distance(first, last);
    std::vector<T> property_vector (num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        NewVertex new_vertex = *first;
        OldVertex old_vertex = getOldVertex(vertex_id_map, new_vertex);
        T value = py::extract<T>(py_property_map.get(old_vertex));
        property_vector[i] = value;
        first++;
    }
    return property_vector;
}

std::vector<int> getVertexPropertyVectorFromPyDict(
    py::dict& py_property_map,
    PCTSPgraph& graph,
    BoostPyBimap& vertex_id_map
);

#endif