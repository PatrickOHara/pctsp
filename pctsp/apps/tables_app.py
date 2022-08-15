"""App for creating LaTeX tables"""

from pathlib import Path
import numpy as np
import pandas as pd
import typer
from tspwplib import LondonaqGraphName
from tspwplib.types import EdgeWeightType
from ..compare import params
from ..lab import Lab
from ..vial import (
    AlgorithmName,
    DatasetName,
    ExperimentName,
    LongAlgorithmName,
    ShortAlgorithmName,
)
from .options import (
    LabDirOption,
    LondonaqRootOption,
    OPLibRootOption,
)

tables_app = typer.Typer(name="tables", help="Write LaTeX tables to files")

PRETTY_COLUMN_NAMES = {
    "algorithm": "Algorithm",
    "alpha": r"$\alpha$",
    "branching_max_depth": r"$\Delta",
    "branching_strategy": "Branching strategy",
    "cost_function": "Cost function",
    "avg_cuts": "AVG CUTS",
    "avg_cuts_presolve": "PRE-CUTS",
    "duration": "TIME",
    "graph_name": "Graph name",
    "kappa": r"$\kappa$",
    "gap": "GAP",
    "max_gap": r"$\max(\text{GAP})$",
    "mean_duration": "TIME",
    "mean_gap": "GAP",
    "min_gap": r"$\min(\text{GAP})$",
    "mean_num_nodes": r"$\mu (NODES)$",
    "mean_num_sec_disjoint_tour": r"$\mu (SEC_DT) $",
    "mean_num_sec_maxflow_mincut": r"$\mu (SEC_MM) $",
    "metricness": r"$\zeta(G, c)$",
    "num_edges": r"$m$",
    "num_nodes": r"$n$",
    "total_cost": r"$c(E(G))$",
    "total_prize": r"$p(V(G))$",
    "num_secs": r"$\mu (SEC)$",
    "num_feasible_solutions": "FEAS",
    "num_optimal_solutions": "OPT",
    "sec_lp_gap_improvement_threshold": r"$\gamma$",
    "sec_max_tailing_off_iterations": r"$\tau$",
    "sec_sepafreq": "SEC freq",
    "std_gap": r"$\sigma (\text{GAP})$",
    "quota": "Quota",
}


def make_column_name_pretty(name: str) -> str:
    """Return a pretty name for the column"""
    if name in PRETTY_COLUMN_NAMES:
        return PRETTY_COLUMN_NAMES[name]
    return name


@tables_app.command(name="dataset")
def summarize_dataset(
    dataset: DatasetName,
    tables_dir: Path,
    londonaq_root: Path = LondonaqRootOption,
    oplib_root: Path = OPLibRootOption,
) -> None:
    """Create a table summarizing each instance of a dataset"""
    if dataset == DatasetName.londonaq:
        filepath = londonaq_root / "londonaq_dataset.csv"
        tables_path = tables_dir / "londonaq_dataset.tex"
    elif dataset == DatasetName.tspwplib:
        filepath = oplib_root / "tsplib_dataset.csv"
        tables_path = tables_dir / "tsplib_dataset.tex"
    df = pd.read_csv(filepath)
    df.style.format(
        {
            "metricness": "{:.2f}",
            "graph_name": lambda x: x[:-1] if x in list(LondonaqGraphName) else x,
        }
    ).format_index(make_column_name_pretty, axis="columns").hide(axis="index").to_latex(
        buf=tables_path,
        hrules=True,
    )


def pretty_dataframe(df: pd.DataFrame, long: bool = False) -> pd.DataFrame:
    """Given a dataframe of results, make the strings and column names 'pretty'

    Args:
        df: The dataframe to make pretty. May contain columns such as 'algorithm'.
        long: If true, use the long version of column names and values.

    Returns:
        A pretty dataframe ready for use in a paper
    """
    if long:
        value_replacements = {
            key.value: LongAlgorithmName[key.name].value for key in AlgorithmName
        }
    else:
        value_replacements = {
            key.value: ShortAlgorithmName[key.name].value for key in AlgorithmName
        }
    # update with cost function names
    value_replacements.update(
        {
            EdgeWeightType.EUC_2D: "EUC",
            EdgeWeightType.MST: "MST",
        }
    )
    # replace values in columns
    for col in ["algorithm", "cost_function", "branching_strategy"]:
        if col in df:
            df = df.replace({col: value_replacements})
        if col in df.index.names:
            df = df.rename(index=value_replacements, level=col)

    # update with branching names
    branching_replacements = {
        0: "Relative branching",
        1: "Strong branching",
        2: "Strong at tree top",
    }
    if "branching_strategy" in df:
        df = df.replace(branching_replacements)
    if "branching_strategy" in df.index.names:
        df = df.rename(index=branching_replacements, level=col)

    # rename columns
    col_replacements = {
        key: value for key, value in PRETTY_COLUMN_NAMES.items() if key in df
    }
    if df.columns.nlevels == 1:
        df = df.rename(columns=col_replacements)
    else:
        for level in range(df.columns.nlevels):
            if df.columns.names[level] == "cc_name":
                pass
            else:
                new_cols = list(
                    map(make_column_name_pretty, df.columns.get_level_values(level))
                )
                df.columns = df.columns.set_levels(
                    new_cols, level=level, verify_integrity=False
                )

    # rename index columns
    if df.index.nlevels == 1:
        name = (
            PRETTY_COLUMN_NAMES[df.index.name]
            if df.index.name in PRETTY_COLUMN_NAMES
            else df.index.name
        )
        df.index = df.index.rename(name)
    else:
        df.index = df.index.rename(list(map(make_column_name_pretty, df.index.names)))

    return df


@tables_app.command(name="tailing-off")
def tailing_off_table(
    dataset: DatasetName,
    tables_dir: Path,
    lab_dir: Path = LabDirOption,
) -> None:
    """Write a table of tailing off experiments to LaTeX file"""
    tables_dir.mkdir(exist_ok=True, parents=False)
    experiment_name = ExperimentName.tailing_off
    stats_lab = Lab(lab_dir / dataset.value)
    filename = (
        stats_lab.get_experiment_dir(experiment_name)
        / f"{dataset.value}_{experiment_name.value}.csv"
    )
    todf: pd.DataFrame = pd.read_csv(filename)  # tailing off dataframe
    todf["gap"] = (todf["upper_bound"] - todf["lower_bound"]) / todf["lower_bound"]
    todf["optimal"] = todf["gap"] == 0
    todf["mean_sec_disjoint_tour_per_node"] = (
        todf["num_sec_disjoint_tour"] / todf["num_nodes"]
    )
    todf["mean_sec_maxflow_mincut_per_node"] = (
        todf["num_sec_maxflow_mincut"] / todf["num_nodes"]
    )
    todf["num_secs"] = (
        todf["mean_sec_disjoint_tour_per_node"]
        + todf["mean_sec_maxflow_mincut_per_node"]
    )

    gb_cols = [
        "branching_max_depth",
        "branching_strategy",
        "sec_lp_gap_improvement_threshold",
        "sec_max_tailing_off_iterations",
    ]
    df = todf.groupby(gb_cols).agg(
        mean_duration=("duration", np.mean),
        mean_gap=("gap", np.mean),
        num_optimal_solutions=("optimal", sum),
        num_feasible_solutions=("feasible", sum),
        mean_num_nodes=("num_nodes", np.mean),
        num_secs=("num_secs", np.mean),
    )
    pretty_df = pretty_dataframe(df)
    print(pretty_df)
    table_tex_filepath = tables_dir / f"{dataset.value}_{experiment_name.value}.tex"
    table_str = pretty_df.to_latex(index=True, float_format="%.2f", escape=False)
    table_tex_filepath.write_text(table_str, encoding="utf-8")


@tables_app.command(name="cost-cover")
def cost_cover_table(
    dataset: DatasetName,
    tables_dir: Path,
    lab_dir: Path = LabDirOption,
) -> None:
    """Write a table of cost cover experiments to LaTeX file"""
    tables_dir.mkdir(exist_ok=True, parents=False)
    experiment_name = ExperimentName.cost_cover
    stats_lab = Lab(lab_dir / dataset.value)
    filename = (
        stats_lab.get_experiment_dir(experiment_name)
        / f"{dataset.value}_{experiment_name.value}.csv"
    )
    ccdf: pd.DataFrame = pd.read_csv(filename)
    ccdf = ccdf.loc[ccdf["cost_function"] != EdgeWeightType.SEMI_MST]
    # create new columns
    ccdf["gap"] = (ccdf["upper_bound"] - ccdf["lower_bound"]) / ccdf["lower_bound"]
    ccdf["optimal"] = ccdf["gap"] == 0
    ccdf["cc_name"] = ccdf["cost_cover_disjoint_paths"].apply(
        lambda x: "Cost cover disjoint paths" if x else "Cost cover shortest paths"
    )

    gb_cols = []
    if dataset == DatasetName.tspwplib:
        ccdf["alpha"] = ccdf["alpha"] / 100
        gb_cols.extend(["cost_function", "alpha"])
    elif dataset == DatasetName.londonaq:
        gb_cols.append("quota")
        ccdf = ccdf.loc[ccdf.graph_name != LondonaqGraphName.laqtinyA]
    gb_cols.append("cc_name")
    ccgb = ccdf.groupby(gb_cols)

    df = ccgb.agg(
        mean_duration=("duration", np.mean),
        mean_gap=("gap", np.mean),
        num_optimal_solutions=("optimal", sum),
        num_feasible_solutions=("feasible", sum),
        num_cost_cover_disjoint_paths=("num_cost_cover_disjoint_paths", np.mean),
        nconss_presolve_disjoint_paths=("nconss_presolve_disjoint_paths", np.mean),
        num_cost_cover_shortest_paths=("num_cost_cover_shortest_paths", np.mean),
        nconss_presolve_shortest_paths=("nconss_presolve_shortest_paths", np.mean),
    )
    df["avg_cuts_presolve"] = (
        df["nconss_presolve_disjoint_paths"] + df["nconss_presolve_shortest_paths"]
    )
    df["avg_cuts"] = (
        df["num_cost_cover_disjoint_paths"] + df["num_cost_cover_shortest_paths"]
    )
    df = df.drop(
        [
            "avg_cuts",
            "num_feasible_solutions",
            "num_cost_cover_disjoint_paths",
            "num_cost_cover_shortest_paths",
            "nconss_presolve_disjoint_paths",
            "nconss_presolve_shortest_paths",
        ],
        axis="columns",
    )
    df = df.unstack()
    df = df.swaplevel(0, 1, axis="columns").sort_index(axis=1)
    df = pretty_dataframe(df)
    table_tex_filepath = tables_dir / f"{dataset.value}_{experiment_name.value}.tex"

    styled_df = df.style.format(
            formatter={
                ("Cost cover disjoint paths", "GAP"): "{:.3f}",
                ("Cost cover shortest paths", "GAP"): "{:.3f}",
                ("Cost cover disjoint paths", "PRE-CUTS"): "{:.2f}",
                ("Cost cover shortest paths", "PRE-CUTS"): "{:.2f}",
                ("Cost cover disjoint paths", "TIME"): "{:.0f}",
                ("Cost cover shortest paths", "TIME"): "{:.0f}",
            },
        )
    if dataset == DatasetName.tspwplib:
        styled_df = styled_df.format_index(formatter={PRETTY_COLUMN_NAMES["alpha"]: "{:.2f}"})
    table_str = styled_df.to_latex(hrules=True, multicol_align="c")
    table_str = table_str.replace("cc_name", "")
    print(table_str)
    table_tex_filepath.write_text(table_str, encoding="utf-8")


def get_heuristics_df(dataset: DatasetName, lab_dir: Path) -> pd.DataFrame:
    """Get a dataframe with the gap between the heuristic solution
    and the lower bounds from an exact algorithm
    """
    heuristic_experiment_name = ExperimentName.compare_heuristics
    exact_experiment_name = ExperimentName.cost_cover

    heuristic_df = pd.read_csv(
        lab_dir
        / dataset.value
        / heuristic_experiment_name.value
        / f"{dataset.value}_{heuristic_experiment_name.value}.csv",
        index_col=0,
    )

    # now read exact experiment to get lower bound
    exact_df = pd.read_csv(
        lab_dir
        / dataset.value
        / exact_experiment_name.value
        / f"{dataset.value}_{exact_experiment_name.value}.csv",
        index_col=0,
    )
    # filter to only get one copy of each instance
    exact_df = exact_df.loc[exact_df["cost_cover_disjoint_paths"]]

    # join on given columns
    join_cols = ["dataset", "graph_name", "quota", "root"]
    if dataset == DatasetName.tspwplib:
        join_cols += ["generation", "kappa", "cost_function"]
    exact_df = exact_df[join_cols + ["lower_bound", "upper_bound"]]
    exact_df = exact_df.drop_duplicates(subset=join_cols)
    heuristic_df = heuristic_df.drop_duplicates(
        subset=join_cols
        + ["algorithm", "path_depth_limit", "step_size", "collapse_paths"]
    )
    heuristic_df = heuristic_df.set_index(join_cols)
    exact_df = exact_df.set_index(join_cols)
    heuristic_df = heuristic_df.join(exact_df)
    heuristic_df["gap"] = (
        heuristic_df["objective"] - heuristic_df["lower_bound"]
    ) / heuristic_df["lower_bound"]
    return heuristic_df


@tables_app.command(name="heuristics")
def heuristics_table(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    tables_dir: Path,
    lab_dir: Path = LabDirOption,
) -> None:
    """Write a table of heuristic performance to a LaTeX file"""
    heuristic_df = get_heuristics_df(dataset, lab_dir)
    heuristic_df = heuristic_df[
        heuristic_df.index.get_level_values("graph_name") != LondonaqGraphName.laqtinyA
    ]

    cols = []
    if dataset == DatasetName.tspwplib:
        cols += ["kappa", "cost_function"]
        heuristic_df = heuristic_df[
            heuristic_df.index.get_level_values("cost_function").isin(
                params.TSPLIB_COST_FUNCTIONS
            )
        ]
    cols.append("algorithm")
    heuristic_gb = heuristic_df.groupby(cols)
    summary_df = heuristic_gb.agg(
        num_feasible_solutions=("feasible", sum),
        # median_gap=("gap", np.median),
        # mean_duration=("duration", np.mean),
    )
    # summary_df = summary_df.reset_index(drop=False)
    if dataset == DatasetName.tspwplib:
        summary_df = summary_df.unstack().unstack()
        summary_df = summary_df.swaplevel(1, 2, axis="columns").sort_index(
            axis="columns"
        )
    # summary_df = pretty_dataframe(summary_df)
    print(summary_df)
    table_tex_filepath = tables_dir / f"{dataset.value}_{experiment_name.value}.tex"
    # table_str = summary_df.to_latex(index=False, float_format="%.2f", escape=False)

    replacements = {
        key.value: ShortAlgorithmName[key.name].value for key in AlgorithmName
    }
    replacements[EdgeWeightType.EUC_2D] = "EUC"

    table_str = summary_df.style.format_index(
        formatter=lambda x: replacements[x] if x in replacements else x, axis="columns"
    ).to_latex(hrules=True, multicol_align="c")
    print(table_str)
    table_tex_filepath.write_text(table_str, encoding="utf-8")


@tables_app.command(name="all")
def generate_all_tables(
    tables_dir: Path,
    lab_dir: Path = LabDirOption,
    londonaq_root: Path = LondonaqRootOption,
    oplib_root: Path = OPLibRootOption,
) -> None:
    """Generate all the LaTeX tables for the paper"""
    for dataset in DatasetName:
        summarize_dataset(
            dataset, tables_dir, londonaq_root=londonaq_root, oplib_root=oplib_root
        )
        cost_cover_table(dataset, tables_dir, lab_dir=lab_dir)
        heuristics_table(
            dataset, ExperimentName.compare_heuristics, tables_dir, lab_dir=lab_dir
        )
        # tailing_off_table(dataset, tables_dir, lab_dir=lab_dir)
