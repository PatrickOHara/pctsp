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

/** Given two vertices, get the cost of the edge between the two vertices */
// template <typename Vertex>
// int getCostOfEdge(PCTSPcostMap &cost_map, Vertex source, Vertex target) {
//     std::pair<Vertex, Vertex> edge = {source, target};
//     if (cost_map.find(edge) == cost_map.end()) {
//         edge = {target, source};
//         if (cost_map.find(edge) == cost_map.end()) {
//             throw EdgeNotFoundException(std::to_string(source),
//                                         std::to_string(target));
//         }
//     }
//     int cost = cost_map[edge];
//     return cost;
// }

/** Given a boost graph, return an adjacency graph representation with
 * internal property maps for prize and cost
 */
// template <typename Graph, typename PrizeMap, typename CostMap>
// void PCTSPgetAdjacencyGraph(Graph &graph, PrizeMap &prize_map,
//                             CostMap &cost_map, PCTSPgraph &adj_graph,
//                             PCTSPprizeMap &std_prize_map,
//                             PCTSPcostMap &std_cost_map) {

//     typedef typename boost::graph_traits<Graph>::vertex_descriptor vd;
//     std::map<vd, int> vertex_id_map;
//     int i = 0;
//     for (auto vertex : make_iterator_range(vertices(graph))) {
//         add_vertex({prize_map[vertex]}, adj_graph);
//         vertex_id_map[vertex] = i;
//         std_prize_map[i] = prize_map[vertex];
//         i++;
//     }
//     for (auto edge : make_iterator_range(edges(graph))) {
//         int source_vertex = vertex_id_map[source(edge, graph)];
//         int target_vertex = vertex_id_map[target(edge, graph)];
//         boost::add_edge(source_vertex, target_vertex, {cost_map[edge]},
//                         adj_graph);
//         auto forward_edge =
//             boost::edge(source_vertex, target_vertex, adj_graph);
//         auto backward_edge =
//             boost::edge(target_vertex, source_vertex, adj_graph);
//         int cost = cost_map[edge];
//         std_cost_map[forward_edge.first] = cost;
//         std_cost_map[backward_edge.first] = cost;
//     }
// }

// template <typename Edge, typename Vertex>
// std::list<Edge>
// toEdgeListFromVertexIdMap(std::map<Vertex, PCTSPvertex> &vertex_id_map,
//                           std::list<PCTSPedge> &edge_list) {
//     std::list<Edge> new_edge_list;

//     return new_edge_list;
// }

// template <typename Graph, typename Vertex, typename BoostPropertyMap>
// void getCostMapFromEdgeProperty(Graph &graph, BoostPropertyMap
// &boost_cost_map,
//                                 PCTSPcostMap<int> &std_cost_map,
//                                 std::map<Vertex, int> &vertex_id_map) {

//     for (auto edge : make_iterator_range(edges(graph))) {
//         int source_vertex = vertex_id_map[source(edge, graph)];
//         int target_vertex = vertex_id_map[target(edge, graph)];
//         std::pair<int, int> std_edge = {source_vertex, target_vertex};
//         std_cost_map[std_edge] = boost_cost_map[edge];
//     }
// }

// template <typename Graph, typename BoostVertexPropertyMap, typename
// Vertex> void getPCTSPprizeMapFromVertexProperty(Graph &graph,
//                                       BoostVertexPropertyMap
//                                       &boost_vertex_map,
//                                       PCTSPprizeMap<Vertex>
//                                       &std_prize_map) {
//     for (Vertex vertex : make_iterator_range(vertices(graph))) {
//         std_prize_map[vertex] = boost_vertex_map[vertex];
//     }
// }
#endif