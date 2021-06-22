#include "fixtures.hh"
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include "pctsp/algorithms.hh"
#include "pctsp/subtour_elimination.hh"
#include <objscip/objscipdefplugins.h>

// ToDo remove debugging code
#include <iostream>

#define SCIP_DEBUG

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
    // SCIP_RETCODE sec_code = addSubtourEliminationConstraint(mip, graph, vertex_set, variable_map, root_vertex, target_vertex);
    // EXPECT_EQ(sec_code, SCIP_OKAY);
    // auto cons_array = SCIPgetConss(mip);
    // // find the subtour elimination constraint by searching for its name
    // for (int i = 0; i < SCIPgetNConss(mip); i++) {
    //     auto cons = cons_array[i];
    //     if (std::string(SCIPconsGetName(cons)).substr(0, 4) == "sec-") {
    //         // the expected num variables = num edges + num vertices - 1
    //         auto nvars = SCIPgetNVarsLinear(mip, cons);
    //         int nedges = getInducedEdges(graph, vertex_set).size();
    //         EXPECT_EQ(nvars, nedges + vertex_set.size() - 1);
    //     }
    // }
    // SCIPsolve(mip);
}

TEST(TestSubtourElimination, testProbDataPCTSP) {
    PCTSPgraph graph;
    SCIP* mip = NULL;
    SCIPcreate(&mip);
    boost::add_edge(0, 1, graph);
    SCIP_VAR* var;
    PCTSPedgeVariableMap edge_variable_map;
    int quota = 2;
    auto root_vertex = boost::vertex(0, graph);
    auto probdata = new ProbDataPCTSP(&graph, &root_vertex, &edge_variable_map, &quota);
    SCIPcreateObjProb(
        mip,
        "test-prob-data",
        probdata,
        true
    );
    SCIPcreateVarBasic(mip, &var, "var0", 0, 1, 1, SCIP_VARTYPE_BINARY);
    auto edge = boost::edge(0, 1, graph).first;
    edge_variable_map[edge] = var;

    // now check the when we get the prob data from the function call that the data is the same
    ProbDataPCTSP* loaddata = dynamic_cast<ProbDataPCTSP*>(SCIPgetObjProbData(mip));
    auto loaded_edge = boost::edge(0, 1, *loaddata->getInputGraph()).first;
    auto loaded_var = (*loaddata->getEdgeVariableMap())[loaded_edge];
    EXPECT_EQ(loaded_edge, edge);
    EXPECT_EQ(loaded_var, var);
    EXPECT_EQ(SCIPvarGetName(var), SCIPvarGetName(loaded_var));
}

typedef GraphFixture SubtourGraphFixture;

TEST_P(SubtourGraphFixture, testPCTSPcreateBasicConsSubtour) {
    PCTSPinitLogging(logging::trivial::debug);
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
    SCIP* mip = NULL;
    SCIPcreate(&mip);
    SCIPincludeObjConshdlr(mip, new PCTSPconshdlrSubtour(mip), TRUE);
    SCIPincludeDefaultPlugins(mip);
    ProbDataPCTSP* probdata = new ProbDataPCTSP(&graph, &root_vertex, &variable_map, &quota);
    SCIPcreateObjProb(
        mip,
        "test-pctsp-with-secs",
        probdata,
        true
    );
    SCIPsetIntParam(mip, "presolving/maxrounds", 0);

    // add variables and constraints
    PCTSPmodelWithoutSECs(mip, graph, cost_map, weight_map, quota,
        root_vertex, variable_map);
    EXPECT_EQ(probdata->getEdgeVariableMap()->size(), variable_map.size());

    // create and add subtour elimination constraint
    SCIP_CONS* cons;
    std::string cons_name("subtour-constraint");
    PCTSPcreateBasicConsSubtour(mip, &cons, cons_name, graph, root_vertex);
    SCIPaddCons(mip, cons);
    SCIPreleaseCons(mip, &cons);
    SCIPprintOrigProblem(mip, NULL, NULL, false);

    SCIPsolve(mip);
    auto sol = SCIPgetBestSol(mip);
    SCIPprintSol(mip, sol, NULL, false);

    auto solution_edges = getSolutionEdges(mip, graph, sol, variable_map);
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
    std::string edge_message = "Edges in solution: ";
    for (auto const& edge : solution_edges)
        edge_message += std::to_string(boost::source(edge, graph)) + "-" + std::to_string(boost::target(edge, graph)) + " ";
    BOOST_LOG_TRIVIAL(debug) << edge_message;
}

INSTANTIATE_TEST_SUITE_P(
    TestSubtourElimination,
    SubtourGraphFixture,
    ::testing::Values(GraphType::GRID8, GraphType::SUURBALLE)
);