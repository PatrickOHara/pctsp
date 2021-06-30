#include "pctsp/separation.hh"
#include <iostream>

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

std::vector<PCTSPedge> getEdgeVectorOfGraph(PCTSPgraph& graph) {
    std::vector<PCTSPedge> edge_vector;
    int i = 0;
    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        edge_vector.insert(edge_vector.begin() + i, edge);
        i++;
    }
    return edge_vector;
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
