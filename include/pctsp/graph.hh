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

typedef adjacency_list_traits< vecS, vecS, directedS > Traits;
typedef boost::property< edge_reverse_t, Traits::edge_descriptor > ReverseEdges;
typedef boost::property< edge_residual_capacity_t, CapacityType, ReverseEdges> ResidualCapacityMap;
typedef boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::directedS,
    boost::no_property,
    property< edge_capacity_t, CapacityType, ResidualCapacityMap>> DirectedCapacityGraph;

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

// lookup table from support vertex to input vertex
typedef std::map<PCTSPvertex, PCTSPvertex> SupportToInputVertexLookup;

SupportToInputVertexLookup getSupportToInputVertexLookup(std::vector<PCTSPvertex>& input_vertices);

SupportToInputVertexLookup getSupportToInputVertexLookupFromEdges(
    StdEdgeVector& input_edge_vector
);

/**
 * @brief Rename vertices to be in the interval [0, num_unique_vertices]
 *
 * @tparam Vertex An integer type (e.g. long, int)
 * @param vertex_vector
 * @return std::map<Vertex, Vertex> Lookup from the new renamed variables to old variable names
 */
template <typename Vertex>
std::map<Vertex, Vertex> renameVertices(
    std::vector<Vertex>& vertex_vector
) {
    std::map<Vertex, Vertex> lookup;
    std::set<Vertex> unique_vertices;
    Vertex i = 0;
    for (Vertex const& vertex : vertex_vector) {
        if (unique_vertices.count(vertex) == 0) {
            lookup[i] = vertex;
            i++;
        }
    }
    return lookup;
}

template <typename Vertex>
std::map<Vertex, Vertex> renameVerticesFromEdges(
    std::vector<std::pair<Vertex, Vertex>>& edge_vector
) {
    std::map<Vertex, Vertex> lookup;
    std::set<Vertex> unique_vertices;
    Vertex i = 0;

    for (auto const& edge : edge_vector) {
        if (unique_vertices.count(edge.first) == 0) {
            unique_vertices.emplace(edge.first);
            lookup.emplace(i, edge.first);
            i++;
        }
        if (unique_vertices.count(edge.second) == 0) {
            unique_vertices.emplace(edge.second);
            lookup.emplace(i, edge.second);
            i++;
        }
    }

    return lookup;
}

#endif