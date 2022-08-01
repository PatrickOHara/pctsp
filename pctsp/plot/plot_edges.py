"""Draw edges with plotly"""

import numpy as np
import pandas as pd
import plotly.graph_objects as go


def edge_scatter(
    edge_df: pd.DataFrame,
    vertex_df: pd.DataFrame,
    source: str = "source",
    target: str = "target",
    x_name: str = "x",
    y_name: str = "y",
    **kwargs,
) -> go.Scatter:
    """Line scatter of edges in a graph"""
    # function to get the x coordinate of the source and target of an edge
    get_x_coord = lambda x: (vertex_df.at[x[0], x_name], vertex_df.at[x[1], x_name])
    get_y_coord = lambda x: (vertex_df.at[x[0], y_name], vertex_df.at[x[1], y_name])

    # stack the source and target into a numpy array
    edge_np = edge_df[[source, target]].to_numpy()

    # apply x/y coordinate mapper and flatten arrays
    edge_x = np.array(list(map(get_x_coord, edge_np))).flatten()
    edge_y = np.array(list(map(get_y_coord, edge_np))).flatten()

    # set the cosmetric params
    edge_cosmetics = dict(
        line=go.scatter.Line(width=0.5, color="#888"),
        hoverinfo="none",
        mode="lines",
    )
    edge_cosmetics.update(kwargs)

    # return a scatter line trace
    return go.Scatter(
        x=edge_x,
        y=edge_y,
        **edge_cosmetics,
    )
