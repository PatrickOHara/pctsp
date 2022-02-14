#ifndef __PCTSP_DATA_STRUCTURES__
#define __PCTSP_DATA_STRUCTURES__

#include "graph.hh"
#include <boost/graph/filtered_graph.hpp>

/** SCIP user problem data for PCTSP */
class ProbDataPCTSP : public scip::ObjProbData
{
    typedef typename boost::graph_traits<PCTSPgraph>::edge_descriptor Edge;
    typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor Vertex;
    typedef std::map<Edge, SCIP_VAR*> EdgeVarLookup;

    int* quota_;
    PCTSPgraph* graph_;
    Vertex* root_vertex_;
    EdgeVarLookup* edge_variable_map_;

public:
    /** default constructor */
    ProbDataPCTSP(
        PCTSPgraph* graph,
        Vertex* root_vertex,
        EdgeVarLookup* edge_variable_map,
        int* quota
    )
    {
        graph_ = graph;
        root_vertex_ = root_vertex;
        edge_variable_map_ = edge_variable_map;
        quota_ = quota;
    };

    /** Get the input graph */
    PCTSPgraph* getInputGraph();

    /** Get the quota */
    int* getQuota();

    /** Get the root vertex */
    Vertex* getRootVertex();

    /** Get the mapping from edges to variables */
    EdgeVarLookup* getEdgeVariableMap();

};

template <typename TGraph>
std::vector<std::vector<typename TGraph::vertex_descriptor>> getConnectedComponentsVectors(
    TGraph& graph,
    int& n_components,
    std::vector<int>& vertex_component_ids
) {
    typedef typename TGraph::vertex_descriptor VertexDescriptor;
    auto v_index = boost::get(vertex_index, graph);
    std::vector<std::vector<VertexDescriptor>> component_vectors(n_components);
    std::vector<int> component_size (n_components);
    std::vector<int> num_vertices_added (n_components);

    for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
        int component_id = vertex_component_ids[v_index[vertex]];
        component_size[component_id] ++;
    }

    for (int i = 0; i < n_components; i++) {
        component_vectors[i] = std::vector<VertexDescriptor>(component_size[i]);
    }
    for (auto vertex: boost::make_iterator_range(boost::vertices(graph))) {
        int component_id = vertex_component_ids[v_index[vertex]];
        component_vectors[component_id][num_vertices_added[component_id]++] = vertex;
    }
    return component_vectors;
}

/**
 * @brief Given a connected components vector of vectors, filter out components
 * that have only one vertex in the component - that is, they are non-singular.
 */
template <typename TVertex>
std::vector<std::vector<TVertex>> getNonSingularConnectedComponentVectors(
    std::vector<std::vector<TVertex>>& component_vectors
) {
    // get the size of each component and the number of non-singular components
    auto n_components = component_vectors.size();
    std::list<int> non_singular_component_ids;
    int n_non_singular_components = 0;
    for (int i = 0; i < n_components; i++) {
        if (component_vectors[i].size() > 1) {
            non_singular_component_ids.push_back(i);
            n_non_singular_components ++;
        }
    }
    // create a new vector of vectors containing non singular components
    std::vector<std::vector<TVertex>> non_singular_components (n_non_singular_components);
    int i = 0;
    for (auto& component_id : non_singular_component_ids) {
        auto& component = component_vectors[i];
        non_singular_components[i] = std::vector<TVertex> (component.size());
        std::copy(component.begin(), component.end(), non_singular_components[i].begin());
        i++;
    }
    return non_singular_components;
}

template <typename TGraph>
std::vector<std::vector<typename TGraph::vertex_descriptor>> getNonSingularConnectedComponentVectors(
    TGraph& graph,
    int& n_components,
    std::vector<int>& vertex_component_ids
) {
    auto component_vectors = getConnectedComponentVectors(graph, n_components, vertex_component_ids);
    return getNonSingularComponentVectors(component_vectors);
}

#endif