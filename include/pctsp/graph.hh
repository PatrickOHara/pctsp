/** A Boost graph defined with a prize vertex property and cost edge property */

#ifndef __PCTSP_GRAPH__
#define __PCTSP_GRAPH__
#include <boost/bimap.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/python.hpp>

using namespace boost;
namespace py = boost::python;

struct PCTSPvertexProperties {
    int prize;
};

struct PCTSPedgeProperties {
    int cost;
};

typedef boost::adjacency_list<listS, vecS, undirectedS, PCTSPvertexProperties,
                              PCTSPedgeProperties>
    PCTSPgraph;

typedef typename boost::graph_traits<PCTSPgraph>::edge_descriptor PCTSPedge;
typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor PCTSPvertex;
typedef typename std::map<PCTSPvertex, int> PCTSPprizeMap;
typedef typename std::map<PCTSPedge, int> PCTSPcostMap;

typedef bimap<int, int> VertexIdMap;
typedef VertexIdMap::value_type position;

PCTSPgraph graphFromPyEdgeList(py::list &edge_list, VertexIdMap &vertex_id_map);
int findOrInsertVertex(PCTSPgraph &graph, VertexIdMap &vertex_id_map,
                       int py_vertex, int &vertex_id);
int getPyVertex(VertexIdMap &vertex_id_map, int vertex);
int getBoostVertex(VertexIdMap &vertex_id_map, int py_vertex);
PCTSPprizeMap prizeMapFromPyDict(py::dict &prize_dict,
                                 VertexIdMap &vertex_id_map);
PCTSPcostMap costMapFromPyDict(py::dict &cost_dict, PCTSPgraph &graph,
                               VertexIdMap &vertex_id_map);
py::list getPyEdgeList(PCTSPgraph &graph, VertexIdMap &vertex_id_map,
                       std::list<PCTSPedge> &edge_list);
py::list getPyVertexList(VertexIdMap &vertex_id_map, std::list<PCTSPvertex> &vertex_list);
std::list<PCTSPvertex> getBoostVertexList(VertexIdMap &vertex_id_map, py::list &py_list);

#endif