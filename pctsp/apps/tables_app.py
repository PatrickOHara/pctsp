"""App for creating LaTeX tables"""

from pathlib import Path
import numpy as np
import pandas as pd
import typer
from tspwplib import LondonaqGraphName
from tspwplib.types import EdgeWeightType
from ..compare import params
from ..lab import Lab
from ..utils import get_pctsp_logger
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
    "branching_max_depth": r"$\Delta$",
    "branching_strategy": "Branching strategy",
    "cost_function": "Cost function",
    "avg_cuts": "AVG CUTS",
    "avg_cuts_presolve": r"$\overline{\text{PRE-CUTS}}$",
    "duration": "TIME (s)",
    "graph_name": "Graph name",
    "kappa": r"$\kappa$",
    "gap": "GAP",
    "max_gap": r"$\max(\text{GAP})$",
    "mean_duration": r"$\overline{\text{TIME}}$ (s)",
    "mean_gap": r"$\overline{\text{GAP}}$",
    "mean_lower_bound": r"$\overline{\text{LB}}$",
    "mean_upper_bound": r"$\overline{\text{UB}}$",
    "min_gap": r"$\min(\\text{GAP})$",
    "mean_num_nodes": r"$\mu (NODES)$",
    "mean_num_sec_disjoint_tour": r"$\mu (SEC_DT) $",
    "mean_num_sec_maxflow_mincut": r"$\mu (SEC_MM) $",
    "metricness": r"$\zeta(G, c)$",
    "num_edges": r"$|E(G)|$",
    "num_nodes": r"$|V(G)|$",
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
    "biggest_disjoint_prize": r"$\max_{t \in G} \{ p(V(\cP_1)) + p(V(\cP_2)) - p(t) \}$",
    "disjoint_prize_ratio": r"$D(G)$",
    "preprocessed_num_nodes": r"$|V(H)|$",
    "preprocessed_num_edges": r"$|E(H)|$",
    "preprocessed_total_cost": r"$c(E(H))",
    "preprocessed_total_prize": r"$p(V(H))$",
    "preprocessed_metricness": r"$\zeta(H, c)$",
    "preprocessed_prize_ratio": r"$\frac{p(V(H))}{p(V(G))}$",
    "mean_preprocessed_prize_ratio": r"\text{AVG}$\left(\frac{p(V(H))}{p(V(G))}\right)$",
    "mean_disjoint_prize_ratio": r"\text{AVG}$(D(G))$",
}

# pylint: disable=line-too-long,too-many-statements
SI_GAP = (
    "S[round-mode=places,round-precision=3,scientific-notation=false,table-format=1.3]"
)
SI_OPT = "S[scientific-notation=false]"
SI_NUM = "S[table-format=1.2e3]"
SI_SEP = "S[round-mode=none,group-separator = {,},group-minimum-digits = 4,scientific-notation=false,table-format=5.0]"

SCIP_STATUS_OPTIMAL = 11
SCIP_STATUS_INFEASIBLE = 12
SCIP_BIG_NUMBER = 100000000000000000000.0


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
    dataset_logger = get_pctsp_logger("dataset")
    if dataset == DatasetName.londonaq:
        filepath = londonaq_root / "londonaq_dataset.csv"
        tables_path = tables_dir / "londonaq_dataset.tex"
        columns = [
            "graph_name",
            "num_nodes",
            "num_edges",
            "preprocessed_num_nodes",
            "preprocessed_num_edges",
            "preprocessed_prize_ratio",
            "metricness",
            "disjoint_prize_ratio",
        ]
        column_format = "l" + 4 * SI_SEP + 3 * SI_GAP
    elif dataset == DatasetName.tspwplib:
        filepath = oplib_root / "tsplib_dataset.csv"
        tables_path = tables_dir / "tsplib_dataset.tex"
        column_format = "l" + 4 * SI_GAP
    dataset_logger.info("Reading dataset CSV from %s", filepath)
    df = pd.read_csv(filepath)
    if dataset == DatasetName.londonaq:
        df = df[columns].set_index("graph_name")
    elif dataset == DatasetName.tspwplib:
        dataset_logger.info("Aggregating dataset stats.")
        df = df.groupby(["cost_function", "kappa"]).aggregate(
            mean_disjoint_prize_ratio=("disjoint_prize_ratio", np.mean),
            mean_preprocessed_prize_ratio=("preprocessed_prize_ratio", np.mean),
        )
        df = (
            df.unstack(level="cost_function")
            .swaplevel(i="cost_function", j=0, axis="columns")
            .sort_index(axis="columns")
        )
    print(df)
    dataset_logger.info("Writing dataset LaTeX table to %s", tables_path)
    df.style.format_index(make_column_name_pretty, axis="columns").format_index(
        make_column_name_pretty, axis="index"
    ).to_latex(
        buf=tables_path,
        hrules=True,
        siunitx=True,
        column_format=column_format,
        multicol_align="c",
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
        4: "Default",
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
    experiment_name: ExperimentName = ExperimentName.cost_cover,
) -> None:
    """Write a table of cost cover experiments to LaTeX file"""
    tables_dir.mkdir(exist_ok=True, parents=False)

    stats_lab = Lab(lab_dir / dataset.value)
    filename = (
        stats_lab.get_experiment_dir(experiment_name)
        / f"{dataset.value}_{experiment_name.value}.csv"
    )
    ccdf: pd.DataFrame = pd.read_csv(filename)
    ccdf = ccdf.loc[ccdf["cost_function"] != EdgeWeightType.SEMI_MST]
    # create new columns
    # see https://www.scipopt.org/doc/html/type__stat_8h_source.php

    def set_to_nan(value: float) -> float:
        return np.nan if value == SCIP_BIG_NUMBER else value

    ccdf["lower_bound"] = ccdf["lower_bound"].apply(set_to_nan)
    ccdf["upper_bound"] = ccdf["upper_bound"].apply(set_to_nan)
    ccdf["gap"] = ccdf.apply(
        lambda x: np.nan
        if x["status"] == SCIP_STATUS_INFEASIBLE
        else (x["upper_bound"] - x["lower_bound"]) / x["lower_bound"],
        axis="columns",
    )
    ccdf["optimal"] = (ccdf["gap"] == 0) & (ccdf["status"] == SCIP_STATUS_OPTIMAL)

    def get_cc_name(cc_disjoint_paths: bool, cc_shortest_paths: bool) -> str:
        if cc_disjoint_paths and not cc_shortest_paths:
            return "Cost cover disjoint paths"
        if not cc_disjoint_paths and cc_shortest_paths:
            return "Cost cover shortest paths"
        if not cc_disjoint_paths and not cc_shortest_paths:
            return "No cost cover"
        raise ValueError("Cannot be both disjoint and shortest paths")

    ccdf["cc_name"] = ccdf[
        ["cost_cover_disjoint_paths", "cost_cover_shortest_path"]
    ].apply(
        lambda x: get_cc_name(x.cost_cover_disjoint_paths, x.cost_cover_shortest_path),
        axis=1,
    )

    gb_cols = []
    if dataset == DatasetName.tspwplib:
        ccdf["alpha"] = ccdf["alpha"] / 100
        gb_cols.extend(["cost_function", "alpha"])
    elif dataset == DatasetName.londonaq:
        if experiment_name == ExperimentName.cc_londonaq_alpha:
            gb_cols.append("alpha")
        elif experiment_name == ExperimentName.cost_cover:
            gb_cols.append("quota")
        ccdf = ccdf.loc[ccdf.graph_name != LondonaqGraphName.laqtinyA]
    gb_cols.append("cc_name")
    ccgb = ccdf.groupby(gb_cols)

    df = ccgb.agg(
        mean_duration=("duration", np.mean),
        mean_lower_bound=("lower_bound", np.mean),
        mean_upper_bound=("upper_bound", np.mean),
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
            "num_cost_cover_disjoint_paths",
            "num_cost_cover_shortest_paths",
            "nconss_presolve_disjoint_paths",
            "nconss_presolve_shortest_paths",
            "avg_cuts",
        ],
        axis="columns",
    )
    if experiment_name == ExperimentName.cc_londonaq_alpha:
        df = df.drop(
            ["num_optimal_solutions", "avg_cuts_presolve", "mean_duration"],
            axis="columns",
        )
        column_format = "l" + 3 * (SI_GAP + SI_NUM + SI_NUM + "r")
    elif experiment_name == ExperimentName.cost_cover:
        df = df.drop(
            ["num_feasible_solutions", "mean_lower_bound", "mean_upper_bound"],
            axis="columns",
        )
        column_format = "l" + 3 * (SI_NUM + SI_NUM + SI_GAP + "r")
    df = df.unstack()
    df = df.swaplevel(0, 1, axis="columns").sort_index(axis=1)
    df = pretty_dataframe(df)
    table_tex_filepath = tables_dir / f"{dataset.value}_{experiment_name.value}.tex"

    # style table using the siunitx package

    if dataset == DatasetName.tspwplib:
        column_format = "l" + column_format
        styled_df = df.style.format_index(
            formatter={PRETTY_COLUMN_NAMES["alpha"]: "{:.2f}"}
        )
    else:
        styled_df = df.style.format()

    table_str = styled_df.to_latex(
        hrules=True, multicol_align="c", siunitx=True, column_format=column_format
    )
    table_str = table_str.replace("cc_name", "")
    print(table_str)
    table_tex_filepath.write_text(table_str, encoding="utf-8")


def get_heuristics_df(
    dataset: DatasetName,
    lab_dir: Path,
    exact_experiment_name=ExperimentName.cost_cover,
    heuristic_experiment_name: ExperimentName = ExperimentName.compare_heuristics,
) -> pd.DataFrame:
    """Get a dataframe with the gap between the heuristic solution
    and the lower bounds from an exact algorithm
    """
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
    heuristic_df["is_optimal"] = heuristic_df["gap"] == 0.0
    return heuristic_df


@tables_app.command(name="heuristics")
def heuristics_table(
    dataset: DatasetName,
    experiment_name: ExperimentName,
    tables_dir: Path,
    lab_dir: Path = LabDirOption,
    exact_experiment_name: ExperimentName = ExperimentName.cost_cover,
    sbl_ec_only: bool = False,
) -> None:
    """Write a table of heuristic performance to a LaTeX file"""
    logger = get_pctsp_logger("heuristics-table")
    heuristic_df = get_heuristics_df(
        dataset,
        lab_dir,
        exact_experiment_name=exact_experiment_name,
        heuristic_experiment_name=experiment_name,
    )

    # NOTE remove SBL-EC heuristic!
    if sbl_ec_only:
        heuristic_df = heuristic_df.loc[
            heuristic_df["algorithm"] == AlgorithmName.suurballes_extension_collapse
        ]
        table_tex_filepath = (
            tables_dir
            / f"{dataset.value}_{experiment_name.value}_{ShortAlgorithmName.suurballes_extension_collapse.value}.tex"
        )
    else:
        heuristic_df = heuristic_df.loc[
            heuristic_df["algorithm"] != AlgorithmName.suurballes_extension_collapse
        ]
        table_tex_filepath = tables_dir / f"{dataset.value}_{experiment_name.value}.tex"
    heuristic_df = heuristic_df[
        heuristic_df.index.get_level_values("graph_name") != LondonaqGraphName.laqtinyA
    ]

    if dataset == DatasetName.tspwplib:
        cols = ["cost_function", "kappa", "algorithm"]
        heuristic_df = heuristic_df[
            heuristic_df.index.get_level_values("cost_function").isin(
                params.TSPLIB_COST_FUNCTIONS
            )
        ]
    elif (
        dataset == DatasetName.londonaq
        and experiment_name == ExperimentName.londonaq_alpha
    ):
        cols = ["alpha", "algorithm"]
    elif dataset == DatasetName.londonaq:
        cols = ["quota", "algorithm"]
    else:
        raise ValueError(f"Dataset {dataset} not recognized.")
    heuristic_gb = heuristic_df.groupby(cols)
    summary_df = heuristic_gb.agg(
        num_optimal_solutions=("is_optimal", sum),
        num_feasible_solutions=("feasible", sum),
        mean_gap=("gap", np.mean),
        mean_duration=("duration", np.mean),
    )
    summary_df = summary_df.unstack(level="algorithm")
    summary_df.columns.rename(["metric", "algorithm"], inplace=True)
    summary_df = summary_df.swaplevel("algorithm", "metric", axis="columns")

    if dataset == DatasetName.londonaq:
        summary_df.index.rename(PRETTY_COLUMN_NAMES["quota"])

    summary_df = summary_df.sort_index(axis="columns")
    summary_df = summary_df.sort_index(axis="index")

    summary_df = summary_df.rename(
        lambda x: ShortAlgorithmName[x], axis="columns", level="algorithm"
    ).rename(PRETTY_COLUMN_NAMES, axis="columns", level="metric")
    summary_df = summary_df.rename(
        {
            EdgeWeightType.EUC_2D: "EUC",
            EdgeWeightType.GEO: "GEO",
            EdgeWeightType.MST: "MST",
        },
        axis="index",
    )

    print(summary_df)
    column_format = "l"
    if dataset == DatasetName.tspwplib:
        column_format += "l"
    column_format += 4 * (SI_NUM + SI_GAP + "r")

    table_str = summary_df.style.to_latex(
        hrules=True,
        multicol_align="c",
        multirow_align="naive",
        siunitx=True,
        column_format=column_format,
    )
    print(table_str)
    logger.info("Writing table to LaTeX file: %s", table_tex_filepath)
    # table_tex_filepath.write_text(table_str, encoding="utf-8")


@tables_app.command(name="all")
def generate_all_tables(
    tables_dir: Path,
    lab_dir: Path = LabDirOption,
    londonaq_root: Path = LondonaqRootOption,
    oplib_root: Path = OPLibRootOption,
) -> None:
    """Generate all the LaTeX tables for the paper"""
    logger = get_pctsp_logger("write-all-tables")
    for dataset in DatasetName:
        try:
            logger.info("Generating dataset tables for %s", dataset.value)
            summarize_dataset(
                dataset, tables_dir, londonaq_root=londonaq_root, oplib_root=oplib_root
            )
        except FileNotFoundError as e:
            logger.warning(str(e))

        try:
            logger.info("Generating cost_cover table on %s dataset", dataset.value)
            cost_cover_table(dataset, tables_dir, lab_dir=lab_dir)
        except FileNotFoundError as e:
            logger.warning(str(e))

        try:
            logger.info("Generating heuristics table on %s dataset", dataset.value)
            heuristics_table(
                dataset, ExperimentName.compare_heuristics, tables_dir, lab_dir=lab_dir
            )
        except FileNotFoundError as e:
            logger.warning(str(e))

        try:
            logger.info("Generating tailing_off tables on %s dataset", dataset.value)
            tailing_off_table(dataset, tables_dir, lab_dir=lab_dir)
        except FileNotFoundError as e:
            logger.warning(str(e))
