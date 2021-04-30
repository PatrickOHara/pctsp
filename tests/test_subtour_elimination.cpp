#include "fixtures.hh"
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include "pctsp/algorithms.hh"
#include "pctsp/subtour_elimination.hh"
#include <objscip/objscipdefplugins.h>

// ToDo remove debugging code
#include <iostream>

TEST_F(SuurballeGraphFixture, testGetEdgesFromCut) {
    PCTSPgraph graph = get_suurballe_graph();
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);

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

    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    BOOST_AUTO(parities, boost::make_one_bit_color_map(num_vertices(graph), get(boost::vertex_index, graph)));

    auto cost_of_cut = stoer_wagner_min_cut(graph, cost_map, boost::parity_map(parities));
    EXPECT_EQ(cost_of_cut, 0);
    auto edges_in_cut = getEdgesFromCut(graph, parities);
    EXPECT_EQ(edges_in_cut.size(), 0);
}

TEST(TestSubtourElimination, testInsertEdgeVertexVariables) {
    // create linear program
    SCIP* mip = NULL;
    SCIPcreate(&mip);
    SCIPincludeDefaultPlugins(mip);
    SCIPcreateProbBasic(mip, "test-insert-edge-vertex-variables");

    int n_edge_vars = 4;
    int n_vertex_vars = 2;
    int nvars = n_edge_vars + n_vertex_vars;

    VarVector edge_variables(n_edge_vars);
    VarVector vertex_variables(n_vertex_vars);
    VarVector all_variables(nvars);
    std::vector<double> var_coefs(nvars);

    for (int i = 0; i < n_edge_vars; i++) {
        SCIP_VAR* var = NULL;
        SCIPcreateVarBasic(mip, &var, NULL, 0, 1, 1, SCIP_VARTYPE_CONTINUOUS);
        SCIPaddVar(mip, var);
        edge_variables[i] = var;
    }
    for (int i = 0; i < n_vertex_vars; i++) {
        SCIP_VAR* var = NULL;
        SCIPcreateVarBasic(mip, &var, NULL, 0, 1, 1, SCIP_VARTYPE_CONTINUOUS);
        SCIPaddVar(mip, var);
        vertex_variables[i] = var;
    }
    // run insertion algorithm
    insertEdgeVertexVariables(edge_variables, vertex_variables, all_variables, var_coefs);
    EXPECT_EQ(edge_variables.size(), n_edge_vars);
    EXPECT_EQ(vertex_variables.size(), n_vertex_vars);
    EXPECT_EQ(all_variables.size(), nvars);
    EXPECT_EQ(var_coefs.size(), nvars);
    for (int i = 0; i < n_edge_vars; i++) {
        EXPECT_EQ(var_coefs[i], 1);
        EXPECT_EQ(edge_variables[i], all_variables[i]);
    }
    for (int i = n_edge_vars; i < nvars; i++) {
        EXPECT_EQ(var_coefs[i], -1);
        EXPECT_EQ(vertex_variables[i - n_edge_vars], all_variables[i]);
    }

}

TEST_F(SuurballeGraphFixture, testAddSubtourEliminationConstraint) {
    PCTSPgraph graph = get_suurballe_graph();
    addSelfLoopsToGraph(graph);
    int quota = 6;
    PCTSPvertex root_vertex = 0;
    auto cost_map = get(&PCTSPedgeProperties::cost, graph);
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);

    std::map<PCTSPedge, SCIP_VAR*> variable_map;
    std::map<PCTSPedge, int> weight_map;
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // initialise and create the model without subtour elimiation constraints
    SCIP* mip = NULL;
    SCIPcreate(&mip);
    SCIPincludeDefaultPlugins(mip);
    SCIPcreateProbBasic(mip, "test-pctsp-without-secs");

    // add variables and constraints
    SCIP_RETCODE code =
        PCTSPmodelWithoutSECs(mip, graph, cost_map, weight_map, quota,
            root_vertex, variable_map);

    // add subtour elimination constraint to the vertex set
    std::vector<PCTSPvertex> vertex_set{ 1, 2, 3, 4 };
    PCTSPvertex target_vertex = 2;
    SCIP_RETCODE sec_code = addSubtourEliminationConstraint(mip, graph, vertex_set, variable_map, root_vertex, target_vertex);
    EXPECT_EQ(sec_code, SCIP_OKAY);
    SCIPsolve(mip);
}