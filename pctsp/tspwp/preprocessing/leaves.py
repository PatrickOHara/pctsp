"""Preprocessing to remove leaves"""

import graph_tool as gt


def remove_leaves(graph: gt.Graph) -> gt.Graph:
    """Remove leaves from the graph

    Args:
        graph: Undirected graph

    Returns:
        New graph with no leaves
    """
    degree_property_map: gt.VertexPropertyMap = graph.degree_property_map("total")
    return gt.Graph(gt.GraphView(graph, vfilt=degree_property_map.get_array() >= 2))
