"""Draw a graph with plotly"""

from typing import List
import pandas as pd
import plotly.graph_objects as go

from .plot_edges import edge_scatter
from .plot_vertices import vertex_scatter


def graph_trace(edge_df: pd.DataFrame, vertex_df: pd.DataFrame) -> List[go.Scatter]:
    """Draw graph with plotly"""
    return [
        edge_scatter(edge_df, vertex_df),
        vertex_scatter(vertex_df),
    ]


def graph_layout() -> go.Layout:
    """Plotly layout of graph"""
    return go.Layout(
        title="<br>Network graph made with Python",
        titlefont_size=16,
        showlegend=False,
        hovermode="closest",
        margin=dict(b=20, l=5, r=5, t=40),
        xaxis=dict(showgrid=False, zeroline=False, showticklabels=False),
        yaxis=dict(showgrid=False, zeroline=False, showticklabels=False),
    )
