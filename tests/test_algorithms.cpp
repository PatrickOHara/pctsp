/** Tests for exact algorithms for PCTSP */

#include "fixtures.hh"
#include "pctsp/graph.hh"
#include "pctsp/algorithms.hh"

typedef GraphFixture AlgorithmsFixture;
typedef GraphFixture SuurballeGraphFixture;

/** Test edge variables are added to the model */
TEST_P(SuurballeGraphFixture, testPCTSPaddEdgeVariables) {
    PCTSPgraph graph = getGraph();
    auto cost_map = getCostMap(graph);
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

TEST_P(SuurballeGraphFixture, testPCTSPwithoutSECs) {
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);
    addSelfLoopsToGraph(graph);
    int quota = 5;
    PCTSPgraph::vertex_descriptor root_vertex = 0;

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

TEST_P(AlgorithmsFixture, testSolvePrizeCollectingTSP) {
    PCTSPgraph graph = getGraph();
    auto prize_map = getPrizeMap(graph);
    auto cost_map = getCostMap(graph);
    int quota = getQuota();
    PCTSPvertex root_vertex = boost::vertex(0, graph);
    addSelfLoopsToGraph(graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    typedef typename boost::graph_traits<PCTSPgraph>::vertex_descriptor vd;
    std::vector<vd> tour;
    switch (GetParam()) {
        case GraphType::GRID8: tour = { 0, 1, 4, 6, 7, 5, 3, 2, 0 }; break;
        case GraphType::SUURBALLE: tour = {0, 1, 3, 6, 7, 5, 2, 0}; break;
        default: tour = {0, 1, 3, 2, 0}; break;
    }
    auto first = tour.begin();
    auto last = tour.end();
    std::vector<PCTSPedge> heuristic_edges = getEdgesInWalk(graph, first, last);
    std::string name = "test-branch-and-cut";
    std::filesystem::path log_dir = ".logs";

    SCIP* scip = NULL;
    SCIPcreate(&scip);
    auto pairs = solvePrizeCollectingTSP(
        scip,
        graph,
        heuristic_edges,
        cost_map,
        prize_map,
        quota,
        root_vertex,
        false,
        false,
        false,
        {},
        name,
        true,
        true,
        log_dir,
        60
    );
    auto solution_edges = edgesFromVertexPairs(graph, pairs);
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
    int quota = getQuota();
    PCTSPvertex root_vertex = getRootVertex();
    auto cost_map = getCostMap(graph);
    auto prize_map = getPrizeMap(graph);
    assignZeroCostToSelfLoops(graph, cost_map);

    std::vector<PCTSPvertex> tour;
    switch (GetParam()) {
        case GraphType::GRID8: tour = { 0, 1, 4, 6, 7, 5, 3, 2, 0 }; break;
        case GraphType::SUURBALLE: tour = {0, 1, 3, 6, 7, 5, 2, 0}; break;
        default: tour = {0, 1, 3, 2, 0}; break;
    }
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

TEST_P(AlgorithmsFixture, testModelPrizeCollectingTSP) {
    auto graph = getGraph();
    auto quota = getQuota();
    auto root_vertex = getRootVertex();
    auto cost_map = getCostMap(graph);
    auto prize_map = getPrizeMap(graph);

    SCIP* scip = NULL;
    SCIPcreate(&scip);
    std::vector<PCTSPvertex> tour;
    switch (GetParam()) {
        case GraphType::GRID8: tour = { 0, 1, 4, 6, 7, 5, 3, 2, 0 }; break;
        case GraphType::SUURBALLE: tour = {0, 1, 3, 6, 7, 5, 2, 0}; break;
        default: tour = {0, 1, 3, 2, 0}; break;
    }
    auto first_it = tour.begin();
    auto last_it = tour.end();
    auto solution_edges = getEdgesInWalk(graph, first_it, last_it);

    std::string name = "testModelPCTSP";
    modelPrizeCollectingTSP(scip, graph, solution_edges, cost_map, prize_map, quota, root_vertex, name);
    EXPECT_EQ(SCIPgetNVars(scip), boost::num_edges(graph));
}

INSTANTIATE_TEST_SUITE_P(
    TestAlgorithms,
    AlgorithmsFixture,
    ::testing::Values(GraphType::COMPLETE4, GraphType::COMPLETE5, GraphType::GRID8, GraphType::SUURBALLE)
);

INSTANTIATE_TEST_SUITE_P(TestAlgorithms, SuurballeGraphFixture,
    ::testing::Values(GraphType::SUURBALLE)
);
