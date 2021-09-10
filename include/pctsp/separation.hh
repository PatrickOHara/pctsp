#ifndef __PCTSP_SEPARATION__
#define __PCTSP_SEPARATION__

#include "graph.hh"
#include "solution.hh"
#include <boost/graph/connected_components.hpp>
#include <objscip/objscip.h>

const CapacityType FLOW_FLOAT_MULTIPLIER = 1000000;

bool isSimpleCycle(PCTSPgraph& graph, std::vector<PCTSPedge>& edge_vector);

/** Returns true if the graph is a simple cycle */
bool isGraphSimpleCycle(PCTSPgraph& graph, std::vector<int>& component_vector);

typedef boost::property<boost::edge_weight_t, CapacityType> BoostCapacityMap;
typedef boost::adjacency_list <
    boost::vecS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property,
    BoostCapacityMap
>UndirectedCapacityGraph;

UndirectedCapacityGraph capacityGraphFromEdgeVector(
    VertexPairVector& edge_vector,
    CapacityVector& capacity_vector
);

/**
 * @brief For each edge with a LP value greater than zero, add the capacity to the vector
 *
 * @param scip Solver
 * @param graph PCTSP input graph
 * @param sol Solution to an LP or Primal solution
 * @param edge_variable_map Mapping from edges to the variables
 * @return CapacityVector
 */
CapacityVector getCapacityVectorFromSol(
    SCIP* scip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map
);

#endif