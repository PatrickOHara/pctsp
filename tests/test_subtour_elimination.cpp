#include "fixtures.hh"
#include <algorithm>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include "pctsp/algorithms.hh"
#include "pctsp/subtour_elimination.hh"
#include <objscip/objscipdefplugins.h>

// ToDo remove debugging code
#include <iostream>

#define SCIP_DEBUG

typedef GraphFixture SubtourGraphFixture;


TEST_P(SubtourGraphFixture, testGetEdgesFromCut) {
    PCTSPgraph graph = getGraph();
    auto cost_map = getCostMap(graph);

    BOOST_AUTO(parities, boost::make_one_bit_color_map(num_vertices(graph), get(boost::vertex_index, graph)));
    auto cost_of_cut = stoer_wagner_min_cut(graph, cost_map, boost::parity_map(parities));
    auto edges_in_cut = getEdgesFromCut(graph, parities);
    int total_cost = 0;
    for (auto const& edge : edges_in_cut) {
        total_cost += cost_map[edge];
    }
    EXPECT_EQ(total_cost, cost_of_cut);
}

TEST(TestSubtourElimination, testDisconnectedCut) {
    PCTSPgraph graph;
    add_edge(0, 1, { 1 }, graph);
    add_edge(1, 2, { 1 }, graph);
    add_edge(2, 0, { 1 }, graph);
    add_edge(3, 4, { 1 }, graph);
    add_edge(4, 5, { 1 }, graph);
    add_edge(3, 5, { 1 }, graph);

    auto cost_map = get(edge_weight, graph);
    BOOST_AUTO(parities, boost::make_one_bit_color_map(num_vertices(graph), get(boost::vertex_index, graph)));

    auto cost_of_cut = stoer_wagner_min_cut(graph, cost_map, boost::parity_map(parities));
    EXPECT_EQ(cost_of_cut, 0);
    auto edges_in_cut = getEdgesFromCut(graph, parities);
    EXPECT_EQ(edges_in_cut.size(), 0);
}

TEST(TestSubtourElimination, testProbDataPCTSP) {
    PCTSPgraph graph;
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    boost::add_edge(0, 1, graph);
    SCIP_VAR* var;
    PCTSPedgeVariableMap edge_variable_map;
    int quota = 2;
    auto root_vertex = boost::vertex(0, graph);
    auto probdata = new ProbDataPCTSP(&graph, &root_vertex, &edge_variable_map, &quota);
    SCIPcreateObjProb(
        scip,
        "test-prob-data",
        probdata,
        true
    );
    SCIPcreateVarBasic(scip, &var, "var0", 0, 1, 1, SCIP_VARTYPE_BINARY);
    auto edge = boost::edge(0, 1, graph).first;
    edge_variable_map[edge] = var;

    // now check the when we get the prob data from the function call that the data is the same
    ProbDataPCTSP* loaddata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(scip));
    auto loaded_edge = boost::edge(0, 1, *loaddata->getInputGraph()).first;
    auto loaded_var = (*loaddata->getEdgeVariableMap())[loaded_edge];
    EXPECT_EQ(loaded_edge, edge);
    EXPECT_EQ(loaded_var, var);
    EXPECT_EQ(SCIPvarGetName(var), SCIPvarGetName(loaded_var));
}


TEST_P(SubtourGraphFixture, testPCTSPcreateBasicConsSubtour) {
    PCTSPinitLogging(logging::trivial::info);
    PCTSPgraph graph = getGraph();
    addSelfLoopsToGraph(graph);
    int quota = getQuota();
    PCTSPvertex root_vertex = 0;
    auto cost_map = getCostMap(graph);
    auto prize_map = getPrizeMap(graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    std::map<PCTSPedge, SCIP_VAR*> variable_map;
    std::map<PCTSPedge, int> weight_map;
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // initialise and create the model without subtour elimiation constraints
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeObjConshdlr(scip, new PCTSPconshdlrSubtour(scip, true, true), TRUE);
    SCIPincludeDefaultPlugins(scip);
    ProbDataPCTSP* probdata = new ProbDataPCTSP(&graph, &root_vertex, &variable_map, &quota);
    SCIPcreateObjProb(
        scip,
        "test-pctsp-with-secs",
        probdata,
        true
    );
    SCIPsetIntParam(scip, "presolving/maxrounds", 0);

    // add custom message handler
    SCIP_MESSAGEHDLR* handler;
    SCIPcreateMessagehdlrDefault(&handler, false, ".logs/test-create-basic_subtour.txt", true);
    SCIPsetMessagehdlr(scip, handler);

    // add variables and constraints
    PCTSPmodelWithoutSECs(scip, graph, cost_map, weight_map, quota,
        root_vertex, variable_map);
    EXPECT_EQ(probdata->getEdgeVariableMap()->size(), variable_map.size());

    // create and add subtour elimination constraint
    SCIP_CONS* cons;
    std::string cons_name("subtour-constraint");
    PCTSPcreateBasicConsSubtour(scip, &cons, cons_name, graph, root_vertex);
    SCIPaddCons(scip, cons);
    SCIPreleaseCons(scip, &cons);

    SCIPsolve(scip);
    auto sol = SCIPgetBestSol(scip);
    auto solution_edges = getSolutionEdges(scip, graph, sol, variable_map);
    auto first_edge = boost::edge(3, 5, graph).first;
    auto second_edge = boost::edge(1, 4, graph).first;
    auto first_it = std::find(solution_edges.begin(), solution_edges.end(), first_edge);
    auto second_it = std::find(solution_edges.begin(), solution_edges.end(), second_edge);
    switch (GetParam()) {
    case GraphType::GRID8: {
        EXPECT_NE(first_it, solution_edges.end());
        EXPECT_NE(second_it, solution_edges.end());
        break;
    }
    case GraphType::SUURBALLE: EXPECT_EQ(*second_it, second_edge); break;
    default: EXPECT_TRUE(true); break;
    }
}

TEST_P(SubtourGraphFixture, testSubtourParams) {
    PCTSPinitLogging(logging::trivial::warning);
    bool sec_disjoint_tour = true;
    double sec_lp_gap_improvement_threshold = 0.01;
    bool sec_maxflow_mincut = true;
    int sec_max_tailing_off_iterations = -1;
    int sec_sepafreq = 1;
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);
    auto root_vertex = getRootVertex();
    int quota;
    switch (GetParam()) {
        case GraphType::COMPLETE50: quota = totalPrizeOfGraph(graph, prize_map); break;
        default: quota = 3; break;
    }

    addSelfLoopsToGraph(graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    std::vector<PCTSPedge> heuristic_edges;
    std::filesystem::path logger_dir = ".logs";

    SCIP* scip = NULL;
    SCIPcreate(&scip);
    std::string name = "testSubtourParams";

    auto solution_edges = solvePrizeCollectingTSP(
        scip,
        graph,
        heuristic_edges,
        cost_map,
        prize_map,
        quota,
        root_vertex,
        -1,
        BranchingStrategy::RELPSCOST,
        false,
        false,
        false,
        {},
        name,
        sec_disjoint_tour,
        sec_lp_gap_improvement_threshold,
        sec_maxflow_mincut,
        sec_max_tailing_off_iterations,
        sec_sepafreq,
        logger_dir,
        60
    );
    auto sol_edges = edgesFromVertexPairs(graph, solution_edges);
    int actual_cost = totalCost(sol_edges, cost_map);
    int expected_cost;
    int expected_num_sec_maxflow_mincut = 0;
    int expected_num_sec_disjoint_tour = 0;
    switch (GetParam()) {
        case GraphType::COMPLETE4: {
            expected_cost = 4;
            break;
        }
        case GraphType::COMPLETE5: {
            expected_cost = 5;
            break;
        }
        case GraphType::GRID8: {
            expected_cost = 4;
            break;
        }
        case GraphType::SUURBALLE: {
            expected_cost = 15;
            break;
        }
        case GraphType::COMPLETE50: {
            expected_num_sec_maxflow_mincut = 0;
            expected_cost = 212;
            expected_num_sec_disjoint_tour = 11;
            break;
        }
        default: {
            expected_num_sec_maxflow_mincut = 0;
            expected_cost = 0;
            expected_num_sec_disjoint_tour = 0;
            break;
        }
    }
    EXPECT_EQ(expected_cost, actual_cost);
    auto summary_yaml = logger_dir / PCTSP_SUMMARY_STATS_YAML;
    auto stats = readSummaryStatsFromYaml(summary_yaml);
    EXPECT_EQ(stats.num_sec_maxflow_mincut, expected_num_sec_maxflow_mincut);
    EXPECT_EQ(stats.num_sec_disjoint_tour, expected_num_sec_disjoint_tour);
    EXPECT_EQ(SCIPgetNNodes(scip), 1);
}

TEST(TestSubtourElimination, testGetUnreachableVertices) {
    typedef boost::property<boost::edge_weight_t, int> DiEdgeWeight;
    typedef boost::adjacency_list<
        boost::listS, boost::vecS, boost::directedS, boost::no_property, DiEdgeWeight> WeightedDiGraph;
    int n_vertices = 6;
    WeightedDiGraph graph(n_vertices);
    auto weight = boost::get(edge_weight, graph);
    for (int i = 0; i < n_vertices - 3; i++) {
        int w = 1;
        auto edge = boost::add_edge(i, i + 1, w, graph);
    }
    boost::add_edge(n_vertices - 3, n_vertices - 2, 0, graph);
    boost::add_edge(n_vertices - 2, n_vertices - 1, 1, graph);

    auto source = boost::vertex(0, graph);
    auto unreachable = getUnreachableVertices(graph, source, weight);
    EXPECT_EQ(unreachable.size(), 2);
    auto end_it = unreachable.end();
    EXPECT_TRUE(std::find(unreachable.begin(), end_it, source) == end_it);
    EXPECT_TRUE(std::find(unreachable.begin(), end_it, boost::vertex(n_vertices - 1, graph)) != end_it);
}

TEST(TestSubtourElimination, testIsNodeTailingOff) {
    std::list<double> rolling_gaps = {1.8, 1.6, 1.3, 1.2, 1.0};
    double threshold = 1.0;
    int max_iter = 5;
    EXPECT_TRUE(isNodeTailingOff(rolling_gaps, threshold, max_iter));
    threshold = 0.1;
    EXPECT_FALSE(isNodeTailingOff(rolling_gaps, threshold, max_iter));
    max_iter = 10;
    threshold = 1.0;
    EXPECT_FALSE(isNodeTailingOff(rolling_gaps, threshold, max_iter));
}

TEST(TestSubtourElimination, testPushIntoRollingLpGapList) {
    std::list<double> rolling_gaps = {1.8, 1.6, 1.3, 1.2, 1.0};
    int max_iter = 5;
    double gap = 0.8;
    EXPECT_EQ(rolling_gaps.size(), max_iter);
    pushIntoRollingLpGapList(rolling_gaps, gap, max_iter);
    EXPECT_EQ(rolling_gaps.size(), max_iter);
    EXPECT_EQ(rolling_gaps.front(), 1.6);
    EXPECT_EQ(rolling_gaps.back(), gap);
}

TEST_P(SubtourGraphFixture, testTailingOff) {
    PCTSPinitLogging(logging::trivial::warning);
    bool sec_disjoint_tour = true;
    double sec_lp_gap_improvement_threshold = 0.1;  // 10% gap improvement required
    bool sec_maxflow_mincut = true;
    int sec_max_tailing_off_iterations = 3;         // every LP is checked for tailing off
    int sec_sepafreq = 1;
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);
    auto root_vertex = getRootVertex();
    int quota = getQuota();
    // switch (GetParam()) {
    //     case GraphType::COMPLETE50: quota = totalPrizeOfGraph(graph, prize_map); break;
    //     default: quota = 3; break;
    // }

    std::vector<PCTSPedge> heuristic_edges = {};
    std::filesystem::path logger_dir = ".logs";

    SCIP* scip = NULL;
    SCIPcreate(&scip);
    std::string name = "testTailingOff";

    auto solution_edges = solvePrizeCollectingTSP(
        scip,
        graph,
        heuristic_edges,
        cost_map,
        prize_map,
        quota,
        root_vertex,
        -1,
        BranchingStrategy::RELPSCOST,
        false,
        false,
        false,
        {},
        name,
        sec_disjoint_tour,
        sec_lp_gap_improvement_threshold,
        sec_maxflow_mincut,
        sec_max_tailing_off_iterations,
        sec_sepafreq,
        logger_dir,
        60
    );
    int expected_num_sec_disjoint_tour;
    int expected_num_sec_maxflow_mincut;
    int expected_cost;
    int expected_nnodes = 1;
    switch (GetParam()) {
        case GraphType::COMPLETE4: {
            expected_num_sec_disjoint_tour = 0;
            expected_num_sec_maxflow_mincut = 0;
            expected_cost = 6;
            break;
        }
        case GraphType::COMPLETE5: {
            expected_num_sec_disjoint_tour = 0;
            expected_num_sec_maxflow_mincut = 0;
            expected_cost = 7;
            break;
        }
        case GraphType::GRID8: {
            expected_num_sec_disjoint_tour = 0;
            expected_num_sec_maxflow_mincut = 0;
            expected_cost = 14;
            break;
        }
        case GraphType::SUURBALLE: {
            expected_num_sec_disjoint_tour = 0;
            expected_num_sec_maxflow_mincut = 0;
            expected_cost = 20;
            break;
        }
        case GraphType::COMPLETE50: {
            expected_num_sec_disjoint_tour = 0;
            expected_num_sec_maxflow_mincut = 39;
            expected_cost = 73;
            expected_nnodes = 9;
            EXPECT_EQ(61, cost_map[boost::edge(0, 5, graph).first]);
            break;
        }
        default: {
            expected_num_sec_maxflow_mincut = 0;
            expected_num_sec_disjoint_tour = 0;
            expected_cost = 0;
            break;
        }
    }
    auto summary_yaml = logger_dir / PCTSP_SUMMARY_STATS_YAML;
    auto stats = readSummaryStatsFromYaml(summary_yaml);
    auto sol_edges = edgesFromVertexPairs(graph, solution_edges);
    int actual_cost = totalCost(sol_edges, cost_map);
    EXPECT_EQ(expected_cost, actual_cost);
    // EXPECT_EQ(stats.num_sec_maxflow_mincut, expected_num_sec_maxflow_mincut);
    EXPECT_EQ(stats.num_sec_disjoint_tour, expected_num_sec_disjoint_tour);
    EXPECT_EQ(SCIPgetNNodes(scip), expected_nnodes);
}

INSTANTIATE_TEST_SUITE_P(
    TestSubtourElimination,
    SubtourGraphFixture,
    ::testing::Values(GraphType::GRID8, GraphType::SUURBALLE, GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::COMPLETE50)
);