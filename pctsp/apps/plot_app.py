"""Functions for plotting and saving figures"""

from pathlib import Path
from typing import Any
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import pandas as pd
from tspwplib import EdgeWeightType
import typer
from ..vial import DatasetName, ShortAlgorithmName
from .options import LabDirOption
from .tables_app import get_heuristics_df

plot_app = typer.Typer(name="plot", help="Plotting results")

# use colors that can be interpreted by color blind readers
ALGORITHM_COLORMAP = {
    ShortAlgorithmName.bfs_extension_collapse.value: "#0072B2",
    ShortAlgorithmName.bfs_path_extension_collapse.value: "#CC79A7",
    ShortAlgorithmName.suurballes_extension_collapse: "#E69F00",
    ShortAlgorithmName.suurballes_heuristic.value: "#009E73",
    ShortAlgorithmName.suurballes_path_extension_collapse: "black",
}


@plot_app.command(name="heuristics")
def plot_heuristics_figure(
    figures_dir: Path,
    lab_dir: Path = LabDirOption,
) -> None:
    """Plot a figure showing the performance of heuristics on a dataset"""
    figures_dir.mkdir(exist_ok=True, parents=False)
    tspwplib_df = get_heuristics_df(DatasetName.tspwplib, lab_dir)
    londonaq_df = get_heuristics_df(DatasetName.londonaq, lab_dir)

    # give short names to algorithms
    tspwplib_df["algorithm"] = tspwplib_df["algorithm"].apply(
        lambda x: ShortAlgorithmName[x]
    )
    londonaq_df["algorithm"] = londonaq_df["algorithm"].apply(
        lambda x: ShortAlgorithmName[x]
    )
    # create subplots for a summary of the heuristic results
    top_fig = make_subplots(
        rows=2,
        cols=2,
        shared_xaxes=True,
        shared_yaxes=False,
        row_heights=[0.9, 0.09],
        horizontal_spacing=0.02,
        vertical_spacing=0.01,
    )
    for algorithm in londonaq_df["algorithm"].unique():
        add_traces_heuristic(
            top_fig,
            londonaq_df,
            algorithm,
            showlegend=True,
            col=1,
        )
        add_traces_heuristic(top_fig, tspwplib_df, algorithm, showlegend=False, col=2)
    update_layout_heuristic(top_fig, col=1, xaxes_title="londonaq")
    update_layout_heuristic(top_fig, col=2, xaxes_title="TSPLIB")
    top_fig.update_layout(
        legend=dict(yanchor="top", y=0.99, xanchor="left", x=0.01),
        autosize=False,
        width=750,
        height=300,
    )
    top_fig.write_image(str(figures_dir / "compare_heuristics.pdf"))

    for cost_function in [EdgeWeightType.MST, EdgeWeightType.EUC_2D]:
        bottom_fig = make_subplots(
            rows=2,
            cols=1,
            shared_xaxes=True,
            shared_yaxes=False,
            row_heights=[0.9, 0.09],
            vertical_spacing=0.01,
        )

        for kappa in tspwplib_df.index.get_level_values("kappa").unique():
            kappa_df = tspwplib_df.iloc[
                tspwplib_df.index.get_level_values("kappa") == kappa
            ]
            kappa_df = kappa_df.iloc[
                kappa_df.index.get_level_values("cost_function") == cost_function
            ]
            for algorithm in [
                ShortAlgorithmName.bfs_extension_collapse,
                ShortAlgorithmName.bfs_path_extension_collapse,
                ShortAlgorithmName.suurballes_extension_collapse,
                ShortAlgorithmName.suurballes_path_extension_collapse,
            ]:
                add_traces_heuristic(
                    bottom_fig,
                    kappa_df,
                    algorithm,
                    showlegend=False,
                    alignmentgroup=kappa,
                )
        update_layout_heuristic(bottom_fig)
        bottom_fig.update_layout(
            autosize=True,
            width=750,
            height=300,
        )
        bottom_fig.write_image(
            str(figures_dir / f"{DatasetName.tspwplib}_{cost_function}_heuristics.pdf")
        )


def add_traces_heuristic(
    fig,
    heuristic_df: pd.DataFrame,
    algorithm: ShortAlgorithmName,
    col: int = 1,
    showlegend: bool = True,
    alignmentgroup: Any = "",
) -> None:
    """Add traces to a box plot showing the gap"""
    algorithm_df = heuristic_df.loc[heuristic_df["algorithm"] == algorithm]
    fig.add_trace(
        go.Box(
            x=[alignmentgroup] * len(algorithm_df),
            y=algorithm_df["gap"],
            alignmentgroup=alignmentgroup,
            offsetgroup=algorithm,
            name=algorithm,
            showlegend=showlegend,
            marker_color=ALGORITHM_COLORMAP[algorithm],
            boxpoints="all",
        ),
        col=col,
        row=1,
    )
    fig.add_trace(
        go.Box(
            x=[alignmentgroup] * len(algorithm_df),
            y=algorithm_df["gap"],
            alignmentgroup=alignmentgroup,
            offsetgroup=algorithm,
            name=algorithm,
            showlegend=False,  # don't show legend for bottom boxes
            marker_color=ALGORITHM_COLORMAP[algorithm],
            boxpoints="all",
        ),
        col=col,
        row=2,
    )


def update_layout_heuristic(
    fig,
    log_max_power: float = 2,
    log_min_power: float = -2,
    col: int = 1,
    xaxes_title: str = r"$\kappa$",
) -> None:
    """Update layout of heuristic"""
    x_range = [-0.5, 4.5]
    fig.update_xaxes(showgrid=False)
    fig.update_xaxes(zeroline=False, col=col, row=1, range=x_range, type="category")
    fig.update_xaxes(
        zeroline=True,
        linewidth=1,
        linecolor="black",
        type="category",
        title=xaxes_title,
        col=col,
        row=2,
        range=x_range,
    )
    fig.update_yaxes(showgrid=False, zeroline=True, linewidth=1, linecolor="black")
    fig.update_yaxes(
        type="log",
        col=col,
        row=1,
        range=[log_min_power, log_max_power],
        showticklabels=col == 1,
        ticks="outside",
    )
    fig.update_yaxes(
        type="linear",
        col=col,
        row=2,
        range=[-0.001, 10**log_min_power - 0.001],
        showticklabels=col == 1,
        ticks="outside",
    )
    fig.update_layout(
        boxmode="group",
        boxgroupgap=0.4,
        boxgap=0.1,
        yaxis_title="GAP",
        plot_bgcolor="rgba(0, 0, 0, 0)",
        paper_bgcolor="rgba(0, 0, 0, 0)",
        margin=dict(l=10, r=10, b=10, t=10, pad=0),
    )
