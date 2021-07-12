#ifndef __PCTSP_PYPCTSP__
#define __PCTSP_PYPCTSP__

#include "graph.hh"
#include "pyutils.hh"
#include "renaming.hh"

// functions for converting between python and boost

PCTSPgraph graphFromPyEdgeList(py::list& edge_list, BoostPyBimap& vertex_id_map);
PCTSPprizeMap prizeMapFromPyDict(py::dict& prize_dict,
    BoostPyBimap& vertex_id_map);
PCTSPcostMap costMapFromPyDict(py::dict& cost_dict, PCTSPgraph& graph,
    BoostPyBimap& vertex_id_map);
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

#endif