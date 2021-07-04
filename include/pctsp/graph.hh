/** A Boost graph defined with a prize vertex property and cost edge property */

#ifndef __PCTSP_GRAPH__
#define __PCTSP_GRAPH__
#include <boost/bimap.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/python.hpp>
#include <objscip/objscip.h>

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
typedef typename std::map<PCTSPedge, SCIP_VAR*> PCTSPedgeVariableMap;

typedef bimap<int, int> VertexIdMap;
typedef VertexIdMap::value_type position;

PCTSPgraph graphFromPyEdgeList(py::list& edge_list, VertexIdMap& vertex_id_map);
int findOrInsertVertex(PCTSPgraph& graph, VertexIdMap& vertex_id_map,
    int py_vertex, int& vertex_id);
int getPyVertex(VertexIdMap& vertex_id_map, int vertex);
int getBoostVertex(VertexIdMap& vertex_id_map, int py_vertex);
PCTSPprizeMap prizeMapFromPyDict(py::dict& prize_dict,
    VertexIdMap& vertex_id_map);
PCTSPcostMap costMapFromPyDict(py::dict& cost_dict, PCTSPgraph& graph,
    VertexIdMap& vertex_id_map);
py::list getPyVertexList(VertexIdMap& vertex_id_map, std::list<int>& vertex_list);
std::list<int> getBoostVertexList(VertexIdMap& vertex_id_map, py::list& py_list);

template <typename EdgeContainer>
py::list getPyEdgeList(PCTSPgraph& graph, VertexIdMap& vertex_id_map,
    EdgeContainer& edge_list) {
    py::list py_list;
    for (auto it = edge_list.begin(); it != edge_list.end(); ++it) {
        PCTSPedge edge = *it;
        int source = boost::source(edge, graph);
        int target = boost::target(edge, graph);
        int py_source = getPyVertex(vertex_id_map, source);
        int py_target = getPyVertex(vertex_id_map, target);
        py::tuple py_edge = py::make_tuple(py_source, py_target);
        py_list.append(py_edge);
    }
    return py_list;
}

typedef unsigned int StdVertex;
typedef std::pair<StdVertex, StdVertex> StdEdge;
typedef std::vector<StdEdge> StdEdgeVector;
typedef std::vector<StdVertex> StdVertexVector;
typedef double CapacityType;
typedef std::vector<CapacityType> CapacityVector;
typedef std::map<StdEdge, CapacityType> StdCapacityMap;
typedef boost::property<boost::edge_weight_t, CapacityType> BoostCapacityMap;
typedef boost::adjacency_list <
    boost::vecS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property,
    BoostCapacityMap
>UndirectedCapacityGraph;

template<typename Graph>
StdEdgeVector getStdEdgeVectorFromGraph(Graph& graph) {
    auto edges = getEdgeVectorOfGraph(graph);
    return getStdEdgeVectorFromEdgeSubset(graph, edges);
}

std::vector <PCTSPedge> getEdgeVectorOfGraph(PCTSPgraph& graph);


StdEdgeVector getStdEdgeVectorFromEdgeSubset(
    PCTSPgraph& graph,
    std::vector < PCTSPedge> edge_subset_vector
);

#endif