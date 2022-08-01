"""Functions for drawing vertices with plotly"""

import pandas as pd
import plotly.graph_objects as go
from tspwplib import VertexFunctionName


def vertex_scatter(
    vertex_df: pd.DataFrame,
    color: str = VertexFunctionName.prize,
    x_name: str = "x",
    y_name: str = "y",
):
    """Scatter plot of vertices"""
    return go.Scatter(
        x=vertex_df[x_name],
        y=vertex_df[y_name],
        mode="markers",
        hoverinfo="text",
        marker=go.scatter.Marker(
            showscale=True,
            colorscale="Viridis",
            reversescale=False,
            size=10,
            color=vertex_df[color],
            colorbar=go.scatter.marker.ColorBar(
                thickness=15, title="Prize", xanchor="left", titleside="right"
            ),
            line_width=2,
        ),
    )
