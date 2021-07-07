/** A Boost graph defined with a prize vertex property and cost edge property */

#ifndef __PCTSP_GRAPH__
#define __PCTSP_GRAPH__
#include <boost/bimap.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <objscip/objscip.h>

using namespace boost;

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
// typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor PCTSPvertex;
typedef unsigned long PCTSPvertex;
typedef typename std::map<PCTSPvertex, int> PCTSPprizeMap;
typedef typename std::map<PCTSPedge, int> PCTSPcostMap;
typedef typename std::map<PCTSPedge, SCIP_VAR*> PCTSPedgeVariableMap;

typedef boost::bimap<PCTSPvertex, int> BoostPyBimap;
typedef boost::bimap<PCTSPvertex, PCTSPvertex> PCTSPbimap;

typedef std::pair<PCTSPvertex, PCTSPvertex> VertexPair;
typedef std::vector<VertexPair> VertexPairVector;
typedef std::vector<PCTSPvertex> PCTSPvertexVector;
typedef long CapacityType;
typedef std::vector<CapacityType> CapacityVector;
typedef std::map<VertexPair, CapacityType> StdCapacityMap;
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


template<typename Graph, typename EdgeIt>
void addEdgesToGraph(Graph& graph, EdgeIt& start, EdgeIt& end) {
    while (std::distance(start, end) > 0) {
        auto edge = *(start);
        boost::add_edge(edge.first, edge.second, graph);
        start++;
    }
}


template<typename Graph>
VertexPairVector getVertexPairVectorFromGraph(Graph& graph) {
    auto edges = getEdgeVectorOfGraph(graph);
    return getVertexPairVectorFromEdgeSubset(graph, edges);
}

std::vector <PCTSPedge> getEdgeVectorOfGraph(PCTSPgraph& graph);


VertexPairVector getVertexPairVectorFromEdgeSubset(
    PCTSPgraph& graph,
    std::vector < PCTSPedge> edge_subset_vector
);

#endif