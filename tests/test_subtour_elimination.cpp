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
    std::vector<NodeStats> node_stats;
    auto probdata = new ProbDataPCTSP(&graph, &root_vertex, &edge_variable_map, &quota, &node_stats);
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
    std::vector<NodeStats> node_stats;

    std::map<PCTSPedge, SCIP_VAR*> variable_map;
    std::map<PCTSPedge, int> weight_map;
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // initialise and create the model without subtour elimiation constraints
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeObjConshdlr(scip, new PCTSPconshdlrSubtour(scip, true, 1, true, 1), TRUE);
    SCIPincludeDefaultPlugins(scip);
    ProbDataPCTSP* probdata = new ProbDataPCTSP(&graph, &root_vertex, &variable_map, &quota, &node_stats);
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
    bool sec_maxflow_mincut = true;
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);
    addSelfLoopsToGraph(graph);
    int quota = 3;
    PCTSPvertex root_vertex = boost::vertex(0, graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    // get filenames
    std::string name = "test-subtour-with-params";
    std::string bounds_filepath = ".logs/" + name + "-bounds.csv";
    std::string log_filepath = ".logs/" + name + ".txt";
    std::string metrics_csv_filepath = ".logs/" + name + "-metrics.csv";
    std::string summary_yaml_filepath = ".logs/" + name + "-summary-stats.yaml";
    std::vector<PCTSPedge> solution_edges;
    SCIP_RETCODE code = PCTSPbranchAndCut(
        graph,
        solution_edges,
        cost_map,
        prize_map,
        quota,
        root_vertex,
        bounds_filepath,
        false,
        false,
        false,
        false,
        {},
        log_filepath,
        metrics_csv_filepath,
        name,
        sec_disjoint_tour,
        1,
        sec_maxflow_mincut,
        1,
        summary_yaml_filepath,
        60
    );
    auto first = solution_edges.begin();
    auto last = solution_edges.end();
    int actual_cost = total_cost(first, last, cost_map);
    int expected_cost;
    switch (GetParam()) {
        case GraphType::GRID8: {
            expected_cost = 4;
            break;
        }
        case GraphType::SUURBALLE: {
            expected_cost = 15;
            break;
        }
        default: {
            // on the complete graphs, no vertices are expected to be above the cost limit
            expected_cost = 0;
            break;
        }
    }
    EXPECT_EQ(expected_cost, actual_cost);
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

INSTANTIATE_TEST_SUITE_P(
    TestSubtourElimination,
    SubtourGraphFixture,
    ::testing::Values(GraphType::GRID8, GraphType::SUURBALLE)
);