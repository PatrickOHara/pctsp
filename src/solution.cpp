#include <iostream>
#include "pctsp/exception.hh"
#include "pctsp/logger.hh"
#include "pctsp/solution.hh"

using namespace std;

std::vector<PCTSPvertex> getSolutionVertices(SCIP* mip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map) {
    std::vector<PCTSPvertex> solution_vertices;
    for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
        auto self_loop = boost::edge(vertex, vertex, graph);
        if (self_loop.second) {
            auto edge = self_loop.first;
            auto var = edge_variable_map[edge];
            auto value = SCIPgetSolVal(mip, sol, var);
            if (value > 0) {
                solution_vertices.push_back(vertex);
            }
        }
        else {
            auto vertex_name = std::to_string(vertex);
            throw EdgeNotFoundException(vertex_name, vertex_name);
        }
    }
    return solution_vertices;
}
std::vector<PCTSPedge> getSolutionEdges(SCIP* mip, PCTSPgraph& graph, SCIP_SOL* sol, std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map, bool add_self_loops) {
    std::vector<PCTSPedge> solution_edges;
    for (auto const& [edge, var] : edge_variable_map) {
        // for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        auto source = boost::source(edge, graph);
        auto target = boost::target(edge, graph);
        if (source != target) {
            auto value = SCIPgetSolVal(mip, sol, var);
            if (!(SCIPisZero(mip, value)) && (value > 0)) {
                solution_edges.push_back(edge);
            }
        }
    }
    return solution_edges;
}

void getSolutionGraph(
    SCIP* mip,
    PCTSPgraph& graph,
    PCTSPgraph& solution_graph,
    SCIP_SOL* sol,
    std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map,
    bool self_loops
) {
    auto solution_edges = getSolutionEdges(mip, graph, sol, edge_variable_map, self_loops);
    // auto solution_vertices = getSolutionVertices(mip, graph, sol, edge_variable_map);
    // PCTSPgraph solution_graph;
    for (auto const& edge : solution_edges) {
        auto source = boost::source(edge, graph);
        auto target = boost::target(edge, graph);
        BOOST_LOG_TRIVIAL(debug) << "Adding " << source << "-" << target;
        if (source != target)
            boost::add_edge(source, target, solution_graph);
    }
}

void logSolutionEdges(
    SCIP* mip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    PCTSPedgeVariableMap& edge_variable_map
) {
    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        auto var = edge_variable_map[edge];
        auto value = SCIPgetSolVal(mip, sol, var);
        if (!(SCIPisZero(mip, value)) && (value > 0)) {
            auto source = boost::source(edge, graph);
            auto target = boost::target(edge, graph);
            auto name = SCIPvarGetName(var);
            BOOST_LOG_TRIVIAL(debug) << "Edge " << source << "-" << target << " has value " << value;
        }
    }
}

SupportToInputVertexLookup getSupportToInputVertexLookup(
    std::vector<PCTSPvertex>& input_vertices
) {
    SupportToInputVertexLookup lookup;
    for (int i = 0; i < input_vertices.size(); i++) {
        lookup[input_vertices[i]] = i;
    }
    return lookup;
}


SupportToInputVertexLookup getSupportToInputVertexLookupFromEdges(
    StdEdgeVector& input_edge_vector
) {
    SupportToInputVertexLookup lookup;
    std::set<StdVertex> unique_vertices;

    for (int i = 0; i < input_edge_vector.size(); i++) {
        StdEdge edge = input_edge_vector[i];
        if (unique_vertices.count(edge.first) == 0) {
            unique_vertices.emplace(edge.first);
            lookup.emplace(i, edge.first);
        }
        if (unique_vertices.count(edge.second) == 0) {
            unique_vertices.emplace(edge.second);
            lookup.emplace(i, edge.second);
        }
    }

    return lookup;
}
