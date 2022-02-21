#include "pctsp/separation.hh"
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/typeof/typeof.hpp>
#include <iostream>
#include <scip/scipdefplugins.h>

void includeSeparation(SCIP* scip) {
    SCIPincludeSepaGomory(scip);
}

bool isGraphSimpleCycle(PCTSPgraph& graph, std::vector<int>& component_vector) {
    if (boost::num_vertices(graph) == 0) return false;
    if (boost::num_edges(graph) == 0) return false;
    int n_components = boost::connected_components(graph, &component_vector[0]);
    if (n_components != 1) return false;    // a simple cycle must be connected

    for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
        if (boost::degree(vertex, graph) != 2) return false;
    }
    return true;
}


bool isSimpleCycle(PCTSPgraph& graph, std::vector<PCTSPedge>& edge_vector) {
    if (edge_vector.size() == 0) return false;

    std::map<PCTSPvertex, std::list<PCTSPvertex>> vertex_count;
    for (auto const& edge : edge_vector) {
        auto source = boost::source(edge, graph);
        auto target = boost::target(edge, graph);
        if (source == target)
            return false;
        if (vertex_count.find(source) == vertex_count.end()) {
            std::list<PCTSPvertex> adj_list = { target };
            vertex_count.emplace(source, adj_list);
        }
        else
            vertex_count[source].emplace_back(target);

        if (vertex_count.find(target) == vertex_count.end()) {
            std::list<PCTSPvertex> adj_list = { source };
            vertex_count.emplace(target, adj_list);
        }
        else
            vertex_count[target].emplace_back(source);

        if (vertex_count[source].size() > 2 || vertex_count[target].size() > 2)
            return false;
    }

    auto edge = edge_vector[0];
    auto source = boost::source(edge, graph);
    auto current = boost::target(edge, graph);
    auto prev = source;
    int i = 0;
    while (current != source && i < edge_vector.size()) {
        auto adj_list = vertex_count[current];
        if (adj_list.size() != 2)
            return false;
        auto it = adj_list.begin();
        PCTSPvertex next;
        if (*it == prev)
            next = *(++it);
        else
            next = *it;
        prev = current;
        current = next;
        i++;
    }
    return i == edge_vector.size() - 1;
}

CapacityVector getCapacityVectorFromSol(
    SCIP* scip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map
) {
    CapacityVector capacity;
    auto edges = getSolutionEdges(scip, graph, sol, edge_variable_map);
    for (auto const& edge : edges) {
        SCIP_VAR* var = edge_variable_map[edge];
        double value = (double)SCIPgetSolVal(scip, sol, var);   // value is less than or equal to 2
        CapacityType int_value;
        if (SCIPisZero(scip, value))
            int_value = 0;
        else if (SCIPisZero(scip, value - 1.0))
            int_value = FLOW_FLOAT_MULTIPLIER;
        else if (SCIPisZero(scip, value - 2.0))
            int_value = FLOW_FLOAT_MULTIPLIER * 2;
        else
            int_value = (CapacityType)(((double)FLOW_FLOAT_MULTIPLIER) * value);
        capacity.push_back(int_value);
    }
    return capacity;
}
