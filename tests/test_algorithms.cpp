/** Tests for exact algorithms for PCTSP */

#include "fixtures.hh"
#include "pctsp/graph.hh"
#include "pctsp/algorithms.hh"

typedef GraphFixture AlgorithmsFixture;


/** Test edge variables are added to the model */
TEST_F(SuurballeGraphFixture, testPCTSPaddEdgeVariables) {
    PCTSPgraph graph = get_suurballe_graph();
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    SCIP* scip_model = NULL;
    SCIPcreate(&scip_model);
    SCIPincludeDefaultPlugins(scip_model);
    SCIPcreateProbBasic(scip_model, "test-add-edge-variables");

    EXPECT_TRUE(scip_model != NULL);

    // declare edge to variable name map
    std::map<PCTSPedge, SCIP_VAR*> variable_map;

    SCIP_RETCODE add_vars_code =
        PCTSPaddEdgeVariables(scip_model, graph, cost_map, variable_map);
    EXPECT_EQ(add_vars_code, SCIP_OKAY);
}

TEST_F(SuurballeGraphFixture, testPCTSPwithoutSECs) {
    PCTSPgraph graph = get_suurballe_graph();
    addSelfLoopsToGraph(graph);
    int quota = 5;
    int root_vertex = 0;
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);

    std::map<PCTSPedge, SCIP_VAR*> variable_map;
    std::map<PCTSPedge, int> weight_map;
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // initialise and create the model without subtour elimiation constraints
    SCIP* scip_model = NULL;
    SCIPcreate(&scip_model);
    SCIPincludeDefaultPlugins(scip_model);
    SCIPcreateProbBasic(scip_model, "test-pctsp-without-secs");

    // add variables and constraints
    SCIP_RETCODE code =
        PCTSPmodelWithoutSECs(scip_model, graph, cost_map, weight_map, quota,
            root_vertex, variable_map);
    EXPECT_EQ(code, SCIP_OKAY);
}

TEST_F(SuurballeGraphFixture, testPCTSPbranchAndCut) {
    PCTSPgraph graph = get_suurballe_graph();
    addSelfLoopsToGraph(graph);
    int quota = 6;
    PCTSPvertex root_vertex = boost::vertex(0, graph);
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor vd;
    std::vector<vd> tour;
    std::vector<int> ids = { 0, 1, 3, 6, 7, 2, 0 };
    for (auto id : ids) {
        tour.push_back(boost::vertex(id, graph));
    }
    auto first = tour.begin();
    auto last = tour.end();
    std::vector<PCTSPedge> solution_edges = getEdgesInWalk(graph, first, last);
    std::string name = "test-branch-and-cut";
    std::string log_filepath = ".logs/" + name + ".txt";
    std::string metrics_csv_filepath = "";
    SCIP_RETCODE code = PCTSPbranchAndCut(graph, solution_edges, cost_map, prize_map,
        quota, root_vertex, log_filepath, metrics_csv_filepath, name, true);
    EXPECT_EQ(SCIP_OKAY, code);
    EXPECT_GT(solution_edges.size(), 0); // check the list is not empty

    // every vertex should be connected to either 2 or 0 edges
    std::map<PCTSPvertex, int> vertex_count;
    for (auto it = solution_edges.begin(); it != solution_edges.end(); it++) {
        PCTSPedge edge = *it;
        PCTSPvertex source = boost::source(edge, graph);
        PCTSPvertex target = boost::target(edge, graph);
        bool source_found = vertex_count.find(source) != vertex_count.end();
        bool target_found = vertex_count.find(target) != vertex_count.end();
        if (source == target && !source_found)
            vertex_count[source] = 0;
        else if (source != target) {
            if (source_found)
                vertex_count[source]++;
            else
                vertex_count[source] = 1;
            if (target_found)
                vertex_count[target]++;
            else
                vertex_count[target] = 1;
        }
    }
    // root vertex should be included in every solution
    EXPECT_EQ(vertex_count[root_vertex], 2);

    // every other vertex should be counted zero or two times
    for (auto const& [vertex, n] : vertex_count) {
        EXPECT_TRUE(n == 0 || n == 2);
    }
}

TEST_P(AlgorithmsFixture, testAddHeuristicVarsToSolver) {
    PCTSPgraph graph = getGraph();
    addSelfLoopsToGraph(graph);
    int quota = 6;
    PCTSPvertex root_vertex = boost::vertex(0, graph);
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    std::vector<PCTSPvertex> tour = { 0, 1, 4, 6, 7, 5, 3, 2, 0 };
    auto first = tour.begin();
    auto last = tour.end();
    std::vector<PCTSPedge> solution_edges = getEdgesInWalk(graph, first, last);

    std::map<PCTSPedge, SCIP_VAR*> variable_map;
    std::map<PCTSPedge, int> weight_map;
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // initialise and create the model without subtour elimiation constraints
    SCIP* scip_model = NULL;
    SCIPcreate(&scip_model);
    SCIPincludeDefaultPlugins(scip_model);
    SCIPcreateProbBasic(scip_model, "test-add-heuristic");

    // add variables and constraints
    SCIP_RETCODE code =
        PCTSPmodelWithoutSECs(scip_model, graph, cost_map, weight_map, quota,
            root_vertex, variable_map);
    auto first_edges = solution_edges.begin();
    auto last_edges = solution_edges.end();
    addHeuristicEdgesToSolver(scip_model, graph, NULL, variable_map, first_edges, last_edges);
}

INSTANTIATE_TEST_SUITE_P(
    TestAlgorithms,
    AlgorithmsFixture,
    ::testing::Values(GraphType::GRID8)
);

