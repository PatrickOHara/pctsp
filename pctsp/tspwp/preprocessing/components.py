"""Preprocess connected components"""

import graph_tool as gt
from graph_tool.topology import label_components
from tspwplib import Vertex


def remove_components_disconnected_from_vertex(
    graph: gt.Graph, vertex: Vertex
) -> gt.Graph:
    """Get a new graph with only one connected component that contains the vertex

    Args:
        graph: Undirected graph to preprocess
        vertex: A vertex in the graph

    Returns:
        Graph containing all vertices connected to the given vertex
    """
    components_property_map: gt.VertexPropertyMap = label_components(graph)[0]
    vertex_component = components_property_map[vertex]
    return gt.Graph(
        gt.GraphView(
            graph, vfilt=components_property_map.get_array() == vertex_component
        )
    )
