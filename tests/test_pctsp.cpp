/** Tests for exact algorithms for PCTSP */

#include "fixtures.hh"
#include "pctsp/graph.hh"
#include "pctsp/algorithms.hh"

/** Test edge variables are added to the model */
TEST_F(SuurballeGraphFixture, testPCTSPaddEdgeVariables) {
    PCTSPgraph graph = get_suurballe_graph();
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    SCIP *scip_model = NULL;
    SCIPcreate(&scip_model);
    SCIPincludeDefaultPlugins(scip_model);
    SCIPcreateProbBasic(scip_model, "test-add-edge-variables");

    EXPECT_TRUE(scip_model != NULL);

    // declare edge to variable name map
    std::map<PCTSPedge, SCIP_VAR *> variable_map;

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

    std::map<PCTSPedge, SCIP_VAR *> variable_map;
    std::map<PCTSPedge, int> weight_map;
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // initialise and create the model without subtour elimiation constraints
    SCIP *scip_model = NULL;
    SCIPcreate(&scip_model);
    SCIPincludeDefaultPlugins(scip_model);
    SCIPcreateProbBasic(scip_model, "test-pctsp-without-secs");

    // add variables and constraints
    SCIP_RETCODE code =
        PCTSPmodelWithoutSECs(scip_model, graph, cost_map, weight_map, quota,
                              root_vertex, variable_map);
    EXPECT_EQ(code, SCIP_OKAY);
    cout << "Done adding SECs" << endl;
}

TEST_F(SuurballeGraphFixture, testPCTSPbranchAndCut) {
    PCTSPgraph graph = get_suurballe_graph();
    addSelfLoopsToGraph(graph);
    int quota = 7;
    int root_vertex = 0;
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);

    std::list<PCTSPedge> edge_list;
    SCIP_RETCODE code = PCTSPbranchAndCut(graph, edge_list, cost_map, prize_map,
                                          quota, root_vertex);
    EXPECT_EQ(SCIP_OKAY, code);
    EXPECT_GT(edge_list.size(), 0); // check the list is not empty

    // every vertex should be connected to either 2 or 0 edges
    std::map<PCTSPvertex, int> vertex_count;
    for (auto it = edge_list.begin(); it != edge_list.end(); it++) {
        PCTSPedge edge = *it;
        PCTSPvertex source = boost::source(edge, graph);
        PCTSPvertex target = boost::target(edge, graph);
        bool source_found = vertex_count.find(source) != vertex_count.end();
        bool target_found = vertex_count.find(target) != vertex_count.end();
        if (source == target && !source_found)
            vertex_count[source] == 0;
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
    for (auto const &[vertex, count] : vertex_count) {
        EXPECT_TRUE(count == 0 || count == 2);
    }
}
