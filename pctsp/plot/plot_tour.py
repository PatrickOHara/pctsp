"""Draw tours on graphs with plotly"""

import pandas as pd
import plotly.graph_objects as go
from tspwplib import VertexList
from tspwplib import edge_list_from_walk
from .plot_edges import edge_scatter


def tour_edges_trace(
    tour: VertexList,
    edge_df: pd.DataFrame,
    vertex_df: pd.DataFrame,
    source: str = "source",
    target: str = "target",
    x_name: str = "x",
    y_name: str = "y",
    color: str = "red",
) -> go.Scatter:
    """Plotly scatter trace of edges in the tour

    Args:
        tour: Cycle that starts and ends at the same vertex
        edge_df: Pandas dataframe of edges and their attributes
        vertex_df: Pandas dataframe of vertices and their attributes (including 'x', 'y')

    Returns:
        Plotly scatter trace of edges
    """
    # set for faster lookup
    tour_edge_set = set(edge_list_from_walk(tour))

    # filter edges
    tour_edge_mask = edge_df[[source, target]].apply(
        lambda x: (x[0], x[1]) in tour_edge_set or (x[1], x[0]) in tour_edge_set, axis=1
    )
    tour_edge_df = edge_df.loc[tour_edge_mask]

    # filter vertices
    tour_vertex_mask = vertex_df.index.isin(set(tour))
    tour_vertex_df = vertex_df[tour_vertex_mask]

    # set line color
    line = go.scatter.Line(width=1, color=color)

    return edge_scatter(
        tour_edge_df,
        tour_vertex_df,
        source=source,
        target=target,
        x_name=x_name,
        y_name=y_name,
        line=line,
    )
