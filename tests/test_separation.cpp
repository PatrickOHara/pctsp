
#include "pctsp/preprocessing.hh"
#include "pctsp/separation.hh"
#include "pctsp/solution.hh"
#include <gtest/gtest.h>
#include <objscip/objscipdefplugins.h>


TEST(TestSeparation, testIsGraphSimpleCycle) {
    PCTSPgraph cycle_graph;
    boost::add_edge(0, 1, cycle_graph);
    boost::add_edge(1, 2, cycle_graph);
    boost::add_edge(2, 3, cycle_graph);
    boost::add_edge(0, 3, cycle_graph);
    std::vector< int > cycle_component(boost::num_vertices(cycle_graph));
    EXPECT_TRUE(isGraphSimpleCycle(cycle_graph, cycle_component));

    PCTSPgraph disjoint_graph;
    boost::add_edge(0, 1, disjoint_graph);
    boost::add_edge(1, 2, disjoint_graph);
    boost::add_edge(0, 2, disjoint_graph);
    boost::add_edge(3, 4, disjoint_graph);
    boost::add_edge(4, 5, disjoint_graph);
    boost::add_edge(3, 5, disjoint_graph);
    std::vector<int> disjoint_component(boost::num_vertices(disjoint_graph));
    EXPECT_FALSE(isGraphSimpleCycle(disjoint_graph, disjoint_component));

    PCTSPgraph path_graph;
    boost::add_edge(0, 1, path_graph);
    boost::add_edge(1, 2, path_graph);
    boost::add_edge(2, 3, path_graph);
    boost::add_edge(3, 4, path_graph);
    std::vector<int> path_component(boost::num_vertices(path_graph));
    EXPECT_FALSE(isGraphSimpleCycle(path_graph, path_component));
}

TEST(TestSeparation, testGetSolutionGraph) {
    PCTSPgraph graph;
    boost::add_edge(0, 1, graph);
    boost::add_edge(1, 2, graph);
    boost::add_edge(2, 3, graph);
    boost::add_edge(3, 4, graph);

    EXPECT_EQ(5, boost::num_vertices(graph));
    EXPECT_EQ(4, boost::num_edges(graph));

    SCIP* mip = NULL;
    SCIPcreate(&mip);
    SCIPincludeDefaultPlugins(mip);
    SCIPcreateProbBasic(mip, "separation");
    SCIPsetObjsense(mip, SCIP_OBJSENSE_MAXIMIZE);

    std::map<boost::graph_traits<PCTSPgraph>::edge_descriptor, SCIP_VAR*> edge_variable_map;
    for (PCTSPedge edge : boost::make_iterator_range(boost::edges(graph))) {
        SCIP_VAR* var;
        SCIPcreateVarBasic(mip, &var, NULL, 0, 1, 1, SCIP_VARTYPE_BINARY);
        SCIPaddVar(mip, var);
        edge_variable_map[edge] = var;
    }
    SCIP_VAR** vars = SCIPgetVars(mip);
    SCIP_CONS* cons = nullptr;
    long long int weights[boost::num_edges(graph)];
    for (int i = 0; i < boost::num_edges(graph); i++) {
        weights[i] = 1;
    }
    SCIPcreateConsBasicKnapsack(mip, &cons, "kp", boost::num_edges(graph), vars, weights, 2);
    SCIPaddCons(mip, cons);
    SCIPreleaseCons(mip, &cons);
    SCIPsolve(mip);
    SCIP_SOL* sol = SCIPgetBestSol(mip);
    // EXPECT_EQ(SCIPgetSolVal(mip, sol, edge_variable_map[boost::edge(0, 1, graph).first]), 0);
    // SCIPsetSolVal(mip, sol, edge_variable_map[boost::edge(0, 1, graph).first], 0.1);
    // EXPECT_EQ(SCIPgetSolVal(mip, sol, edge_variable_map[boost::edge(0, 1, graph).first]), 0.1);

    int objective = 0;
    std::list<PCTSPedge> solution_edges;
    for (auto const& [edge, var] : edge_variable_map) {
        auto value = SCIPgetSolVal(mip, sol, var);
        if (value > 0) {
            objective += value;
            solution_edges.push_back(edge);
        }
    }
    EXPECT_EQ(objective, 2);

    addSelfLoopsToGraph(graph);
    PCTSPgraph solution_graph;
    getSolutionGraph(mip, graph, solution_graph, sol, edge_variable_map);

    EXPECT_EQ(boost::num_edges(solution_graph), objective);
    for (auto const& edge : solution_edges) {
        auto source = boost::source(edge, graph);
        auto target = boost::target(edge, graph);
        EXPECT_TRUE(boost::edge(source, target, solution_graph).second);
    }
}

TEST(TestSeparation, testRemoveIsolatedVertices) {
    PCTSPgraph graph;
    for (int i = 0; i < 5; i++) {
        boost::add_edge(i, i + 1, graph);
    }
    boost::add_vertex(graph);
    boost::add_vertex(graph);
    int n_vertices = boost::num_vertices(graph);

    std::vector< int > component(n_vertices);
    int n_components = boost::connected_components(graph, &component[0]);
    EXPECT_EQ(n_components, 3);

    removeIsolatedVertices(graph);
    EXPECT_EQ(n_vertices - 2, boost::num_vertices(graph));
    std::vector<int> one_component(boost::num_vertices(graph));
    n_components = boost::connected_components(graph, &one_component[0]);
    EXPECT_EQ(n_components, 1);
}
