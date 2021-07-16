#ifndef __PCTSP_RENAME__
#define __PCTSP_RENAME__
#include <boost/bimap.hpp>

// functions for renaming vertices and creating new graphs

template<typename OldVertex, typename NewVertex>
NewVertex findOrInsertVertex(
    boost::bimap<NewVertex, OldVertex>& vertex_bimap,
    OldVertex& old_vertex,
    NewVertex& new_vertex
) {
    typedef typename boost::bimap<NewVertex, OldVertex>::value_type position;
    NewVertex graph_vertex;
    auto it = vertex_bimap.right.find(old_vertex);
    if (it == vertex_bimap.right.end()) {
        vertex_bimap.insert(position(new_vertex, old_vertex));
        graph_vertex = new_vertex;
        new_vertex++;
    }
    else {
        graph_vertex = (*it).second;
    }
    return graph_vertex;
}

template <typename OldVertexContainer, typename OldVertex, typename NewVertex>
void renameVertices(
    boost::bimap<NewVertex, OldVertex>& vertex_bimap,
    OldVertexContainer& vertices
) {
    NewVertex new_vertex = 0;
    for (auto it = vertices.begin(); it != vertices.end(); it++) {
        OldVertex old_vertex = *it;
        findOrInsertVertex(vertex_bimap, old_vertex, new_vertex);
    }
}

template<typename OldEdgeContainer, typename OldVertex, typename NewVertex>
std::vector<std::pair<NewVertex, NewVertex>> renameEdges(
    boost::bimap<NewVertex, OldVertex>& vertex_bimap,
    OldEdgeContainer& old_edges
) {
    std::vector<std::pair<NewVertex, NewVertex>> new_edges(old_edges.size());
    NewVertex new_vertex = 0;
    int i = 0;
    for (auto it = old_edges.begin(); it != old_edges.end(); it++) {
        auto pair = *(it);
        OldVertex old_source = pair.first;
        OldVertex old_target = pair.second;
        NewVertex new_source = findOrInsertVertex(vertex_bimap, old_source, new_vertex);
        NewVertex new_target = findOrInsertVertex(vertex_bimap, old_target, new_vertex);
        std::pair<NewVertex, NewVertex> new_pair(new_source, new_target);
        new_edges[i] = new_pair;
        i++;
    }
    return new_edges;
}

template<typename OldVertex, typename NewVertex>
OldVertex getOldVertex(boost::bimap<NewVertex, OldVertex>& vertex_bimap, NewVertex& new_vertex) {
    auto it = vertex_bimap.left.find(new_vertex);
    return (*it).second;
}

template<typename OldVertex, typename NewVertex>
NewVertex getNewVertex(boost::bimap<NewVertex, OldVertex>& vertex_bimap, OldVertex& old_vertex) {
    auto it = vertex_bimap.right.find(old_vertex);
    return (*it).second;
}

template<typename OldVertex, typename NewVertex>
std::pair<OldVertex, OldVertex> getOldEdge(
    boost::bimap<NewVertex, OldVertex>& vertex_bimap,
    std::pair<NewVertex, NewVertex>& new_edge
) {
    std::pair<OldVertex, OldVertex> old_edge;
    old_edge.first = getOldVertex(vertex_bimap, new_edge.first);
    old_edge.second = getOldVertex(vertex_bimap, new_edge.second);
    return old_edge;
}

template<typename OldVertex, typename NewVertex>
std::pair<NewVertex, NewVertex> getNewEdge(
    boost::bimap<NewVertex, OldVertex>& vertex_bimap,
    std::pair<OldVertex, OldVertex>& old_edge
) {
    std::pair<NewVertex, NewVertex> new_edge;
    new_edge.first = getNewVertex(vertex_bimap, old_edge.first);
    new_edge.second = getNewVertex(vertex_bimap, old_edge.second);
    return new_edge;
}

template<typename NewVertexContainer, typename OldVertex, typename NewVertex>
std::vector<OldVertex> getOldVertices(
    boost::bimap<NewVertex, OldVertex>& vertex_bimap,
    NewVertexContainer& new_vertices
) {
    std::vector<OldVertex> old_vertices(new_vertices.size());
    int i = 0;
    for (auto it = new_vertices.begin(); it != new_vertices.end(); it++) {
        old_vertices[i++] = getOldVertex(vertex_bimap, *it);
    }
    return old_vertices;
}

template<typename OldVertexContainer, typename OldVertex, typename NewVertex>
std::vector<NewVertex> getNewVertices(
    boost::bimap<NewVertex, OldVertex>& vertex_bimap,
    OldVertexContainer& old_vertices
) {
    std::vector<NewVertex> new_vertices(old_vertices.size());
    int i = 0;
    for (auto it = old_vertices.begin(); it != old_vertices.end(); it++) {
        new_vertices[i++] = getNewVertex(vertex_bimap, *it);
    }
    return new_vertices;
}

template<typename NewEdgeContainer, typename OldVertex, typename NewVertex>
std::vector<std::pair<OldVertex, OldVertex>> getOldEdges(
    boost::bimap<NewVertex, OldVertex>& vertex_bimap,
    NewEdgeContainer& new_edges
) {
    std::vector<std::pair<OldVertex, OldVertex>> old_edges(new_edges.size());
    int i = 0;
    for (auto it = new_edges.begin(); it != new_edges.end(); it++) {
        old_edges[i++] = getOldEdge(vertex_bimap, *it);
    }
    return old_edges;
}

template<typename OldEdgeContainer, typename OldVertex, typename NewVertex>
std::vector<std::pair<NewVertex, NewVertex>> getNewEdges(
    boost::bimap<NewVertex, OldVertex>& vertex_bimap,
    OldEdgeContainer& old_edges
) {
    std::vector<std::pair<NewVertex, NewVertex>> new_edges(old_edges.size());
    int i = 0;
    for (auto it = old_edges.begin(); it != old_edges.end(); it++) {
        new_edges[i++] = getNewEdge(vertex_bimap, *it);
    }
    return new_edges;
}

#endif