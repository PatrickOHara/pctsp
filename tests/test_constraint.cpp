#include <gtest/gtest.h>

#include "fixtures.hh"
#include "pctsp/algorithms.hh"

typedef GraphFixture SuurballeGraphFixture;

TEST(TestConstraint, testAddRootVertexConstraint) {
    SCIP *scip_model = NULL;
    SCIPcreate(&scip_model);
    SCIPincludeDefaultPlugins(scip_model);
    SCIPcreateProbBasic(scip_model, "test-pctsp-without-secs");
    typedef typename std::pair<int, int> edge_t;
    edge_t self_loop = {9, 9};
    SCIP_VAR *variable;
    SCIPcreateVarBasic(scip_model, &variable, NULL, 0, 1, 1,
                       SCIP_VARTYPE_BINARY);
    std::map<edge_t, SCIP_VAR *> edge_variable_map;
    edge_variable_map[self_loop] = variable;
    PCTSPaddRootVertexConstraint(scip_model, edge_variable_map, self_loop);
    SCIPreleaseVar(scip_model, &variable);
    EXPECT_EQ(SCIPgetNConss(scip_model), 1);
}

TEST_P(SuurballeGraphFixture, testAddDegreeTwoConstraint) {
    PCTSPgraph graph = getGraph();
    auto cost_map = getCostMap(graph);
    addSelfLoopsToGraph(graph);

    typedef typename PCTSPgraph::edge_descriptor edge_t;
    std::map<edge_t, SCIP_VAR *> edge_variable_map;
    SCIP *scip_model = NULL;
    SCIPcreate(&scip_model);
    SCIPincludeDefaultPlugins(scip_model);
    SCIPcreateProbBasic(scip_model, "test-add-degree-two-constraint");

    PCTSPaddEdgeVariables(scip_model, graph, cost_map, edge_variable_map);
    PCTSPaddDegreeTwoConstraint(scip_model, graph, edge_variable_map);
    EXPECT_EQ(SCIPgetNConss(scip_model), boost::num_vertices(graph));
}

INSTANTIATE_TEST_SUITE_P(TestConstraint, SuurballeGraphFixture,
    ::testing::Values(GraphType::SUURBALLE)
);