
#include <iostream>
#include "pctsp/preprocessing.hh"
#include "pctsp/separation.hh"
#include "pctsp/solution.hh"
#include <gtest/gtest.h>
#include <objscip/objscip.h>
#include <scip/message_default.h>
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

    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);
    SCIPcreateProbBasic(scip, "separation");
    SCIPsetObjsense(scip, SCIP_OBJSENSE_MAXIMIZE);
    SCIP_MESSAGEHDLR* handler;
    SCIPcreateMessagehdlrDefault(&handler, false, ".logs/test-get-solution-graph.txt", true);
    SCIPsetMessagehdlr(scip, handler);


    std::map<boost::graph_traits<PCTSPgraph>::edge_descriptor, SCIP_VAR*> edge_variable_map;
    for (PCTSPedge edge : boost::make_iterator_range(boost::edges(graph))) {
        SCIP_VAR* var;
        SCIPcreateVarBasic(scip, &var, NULL, 0, 1, 1, SCIP_VARTYPE_BINARY);
        SCIPaddVar(scip, var);
        edge_variable_map[edge] = var;
    }
    SCIP_VAR** vars = SCIPgetVars(scip);
    SCIP_CONS* cons = nullptr;
    long long int weights[boost::num_edges(graph)];
    for (int i = 0; i < boost::num_edges(graph); i++) {
        weights[i] = 1;
    }
    SCIPcreateConsBasicKnapsack(scip, &cons, "kp", boost::num_edges(graph), vars, weights, 2);
    SCIPaddCons(scip, cons);
    SCIPreleaseCons(scip, &cons);
    SCIPsolve(scip);
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    // EXPECT_EQ(SCIPgetSolVal(scip, sol, edge_variable_map[boost::edge(0, 1, graph).first]), 0);
    // SCIPsetSolVal(scip, sol, edge_variable_map[boost::edge(0, 1, graph).first], 0.1);
    // EXPECT_EQ(SCIPgetSolVal(scip, sol, edge_variable_map[boost::edge(0, 1, graph).first]), 0.1);

    int objective = 0;
    std::list<PCTSPedge> solution_edges;
    for (auto const& [edge, var] : edge_variable_map) {
        auto value = SCIPgetSolVal(scip, sol, var);
        if (value > 0) {
            objective += value;
            solution_edges.push_back(edge);
        }
    }
    EXPECT_EQ(objective, 2);

    addSelfLoopsToGraph(graph);
    PCTSPgraph solution_graph;
    getSolutionGraph(scip, graph, solution_graph, sol, edge_variable_map);

    EXPECT_EQ(boost::num_edges(solution_graph), objective);
    for (auto const& edge : solution_edges) {
        auto source = boost::source(edge, graph);
        auto target = boost::target(edge, graph);
        EXPECT_TRUE(boost::edge(source, target, solution_graph).second);
    }
}

TEST(TestSeparation, testIsSimpleCycle) {
    PCTSPgraph graph1;
    for (int i = 0; i < 5; i++) {
        boost::add_edge(i, i + 1, graph1);
    }
    auto edge_vector1 = getEdgeVectorOfGraph(graph1);
    EXPECT_FALSE(isSimpleCycle(graph1, edge_vector1));

    boost::add_edge(0, boost::num_vertices(graph1) - 1, graph1);
    std::vector<int> cv(boost::num_vertices(graph1));
    EXPECT_TRUE(isGraphSimpleCycle(graph1, cv));
    auto edge_vector2 = getEdgeVectorOfGraph(graph1);
    EXPECT_TRUE(isSimpleCycle(graph1, edge_vector2));

    auto edge23 = boost::edge(2, 3, graph1).first;
    auto edge05 = boost::edge(0, 5, graph1).first;
    boost::remove_edge(edge23, graph1);
    boost::remove_edge(edge05, graph1);
    boost::add_edge(0, 2, graph1);
    boost::add_edge(3, 5, graph1);
    auto edge_vector3 = getEdgeVectorOfGraph(graph1);
    EXPECT_FALSE(isSimpleCycle(graph1, edge_vector3));
}
