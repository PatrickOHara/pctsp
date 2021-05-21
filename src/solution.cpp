#include <iostream>
#include "pctsp/exception.hh"
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
    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        std::cout << "Get source: ";
        auto source = boost::source(edge, graph);
        std::cout << source << ". Target: ";
        auto target = boost::target(edge, graph);
        std::cout << target << endl;
        // if (((source == target) & (add_self_loops)) || (source != target)) {
        if (source != target) {
            std::cout << "Source not equal to target" << endl;
            std::cout << "Get variable: ";
            auto var = edge_variable_map[edge];
            std::cout << " and it's value: ";
            auto value = SCIPgetSolVal(mip, sol, var);
            std::cout << value << ". Then push to back of vector..." << endl;
            solution_edges.push_back(edge);
        }
        else
            std::cout << "Source equal to target" << endl;
    }
    std::cout << solution_edges.size() << " edges added to vector." << endl;
    return solution_edges;
}

PCTSPgraph getSolutionGraph(
    SCIP* mip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map,
    bool self_loops
) {
    std::cout << "Get solution edges" << endl;
    auto solution_edges = getSolutionEdges(mip, graph, sol, edge_variable_map, self_loops);
    // auto solution_vertices = getSolutionVertices(mip, graph, sol, edge_variable_map);
    std::cout << "Get solution graph. Add edges." << endl;
    PCTSPgraph solution_graph;
    for (auto const& edge : solution_edges) {
        auto source = boost::source(edge, graph);
        auto target = boost::target(edge, graph);
        if (source != target)
            boost::add_edge(source, target, solution_graph);
    }
    return solution_graph;
}
