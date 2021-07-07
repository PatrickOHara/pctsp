/** Test getting the adjacency grap*/

#include "pctsp/graph.hh"
#include "fixtures.hh"
#include <gtest/gtest.h>

TEST(TestGraph, testGraphFromPyEdgeList) {
    Py_Initialize();
    py::tuple edge = py::make_tuple(0, 1);
    EXPECT_EQ(py::extract<int>(edge[0]), 0);
    py::list edge_list;
    edge_list.append(edge);
    VertexIdMap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    EXPECT_EQ(py::len(edge_list), boost::num_edges(graph));
    EXPECT_TRUE(boost::edge(0, 1, graph).second);
    EXPECT_TRUE(boost::edge(1, 0, graph).second);
}

TEST(TestGraph, testPrizeMapFromPyDict) {
    Py_Initialize();
    py::tuple prize1 = py::make_tuple(0, 1);
    py::tuple prize2 = py::make_tuple(2, 5);
    py::list prize_list;
    prize_list.append(prize1);
    prize_list.append(prize2);
    py::dict prize_dict(prize_list);
    EXPECT_EQ(py::len(prize_list), py::len(prize_list));

    VertexIdMap vertex_id_map;
    vertex_id_map.insert(position(0, 0));
    vertex_id_map.insert(position(1, 2));

    PCTSPprizeMap prize_map = prizeMapFromPyDict(prize_dict, vertex_id_map);

    EXPECT_EQ(prize_map[0], 1);
    EXPECT_EQ(prize_map[1], 5);
}

TEST(TestGraph, testGetPyVertex) {
    VertexIdMap vertex_id_map;
    vertex_id_map.insert(position(0, 1));
    vertex_id_map.insert(position(1, 2));

    EXPECT_EQ(getPyVertex(vertex_id_map, 0), 1);
    EXPECT_EQ(getPyVertex(vertex_id_map, 1), 2);
}

TEST(TestGraph, testGetBoostVertex) {
    VertexIdMap vertex_id_map;
    vertex_id_map.insert(position(0, 1));
    vertex_id_map.insert(position(1, 2));

    EXPECT_EQ(getBoostVertex(vertex_id_map, 1), 0);
    EXPECT_EQ(getBoostVertex(vertex_id_map, 2), 1);
}

TEST(TestGraph, testGetCostMapFromPyDict) {
    Py_Initialize();

    // create edges and append to edge list
    py::tuple edge1 = py::make_tuple(2, 4);
    py::tuple edge2 = py::make_tuple(1, 2);
    py::list edge_list;
    edge_list.append(edge1);
    edge_list.append(edge2);

    // create dictionary of edge costs
    py::list cost_list;
    py::tuple cost1 = py::make_tuple(edge1, 5);
    py::tuple cost2 = py::make_tuple(edge2, 7);
    cost_list.append(cost1);
    cost_list.append(cost2);
    py::dict cost_dict(cost_list);

    // get the graph and vertex lookup
    VertexIdMap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);

    // get the std::cost map and check the costs are correct
    PCTSPcostMap cost_map = costMapFromPyDict(cost_dict, graph, vertex_id_map);
    PCTSPedge boost_edge1 = boost::edge(getBoostVertex(vertex_id_map, 2),
        getBoostVertex(vertex_id_map, 4), graph)
        .first;
    PCTSPedge boost_edge2 = boost::edge(getBoostVertex(vertex_id_map, 1),
        getBoostVertex(vertex_id_map, 2), graph)
        .first;
    EXPECT_EQ(cost_map[boost_edge1], 5);
    EXPECT_EQ(cost_map[boost_edge2], 7);
}

TEST(TestGraph, testGetPyEdgeList) {
    PCTSPgraph graph;
    PCTSPedge edge1 = boost::add_edge(0, 1, graph).first;
    PCTSPedge edge2 = boost::add_edge(1, 2, graph).first;

    VertexIdMap vertex_id_map;
    vertex_id_map.insert(position(0, 7));
    vertex_id_map.insert(position(1, 3));
    vertex_id_map.insert(position(2, 5));

    std::list<PCTSPedge> edge_list = { edge1, edge2 };
    py::list py_edge_list = getPyEdgeList(graph, vertex_id_map, edge_list);    EXPECT_EQ(py::len(py_edge_list), edge_list.size());
    EXPECT_TRUE(py_edge_list.contains(py::make_tuple(7, 3)));
}

TEST(TestGraph, testGetPyVertexList) {
    std::list<int> vertex_list = { 0, 1, 2 };
    VertexIdMap vertex_id_map;
    vertex_id_map.insert(position(0, 7));
    vertex_id_map.insert(position(1, 3));
    vertex_id_map.insert(position(2, 5));
    py::list py_list = getPyVertexList(vertex_id_map, vertex_list);
    EXPECT_EQ(py_list[0], 7);
    EXPECT_EQ(py_list[1], 3);
    EXPECT_EQ(py_list[2], 5);
}

TEST(TestGraph, testGetBoostVertexList) {
    py::list py_list;
    py_list.append(7);
    py_list.append(3);
    py_list.append(5);
    VertexIdMap vertex_id_map;
    vertex_id_map.insert(position(0, 7));
    vertex_id_map.insert(position(1, 3));
    vertex_id_map.insert(position(2, 5));
    std::list<int> vertex_list = getBoostVertexList(vertex_id_map, py_list);
    auto it = vertex_list.begin();
    EXPECT_EQ(*(it++), 0);
    EXPECT_EQ(*(it++), 1);
    EXPECT_EQ(*(it), 2);
}

typedef GraphFixture EdgeSubsetGraph;

TEST_P(EdgeSubsetGraph, testGetVertexPairVectorFromEdgeSubset) {
    PCTSPgraph graph = getGraph();
    std::vector<PCTSPedge> edge_subset;
    for (auto edge : boost::make_iterator_range(boost::edges(graph))) {
        if (boost::source(edge, graph) == 0)
            edge_subset.push_back(edge);
        if (boost::target(edge, graph) == 0)
            edge_subset.push_back(edge);
    }
    auto edge_vector = getVertexPairVectorFromEdgeSubset(graph, edge_subset);
    EXPECT_GE(edge_vector.size(), 2);
    for (auto const& pair : edge_vector) {
        EXPECT_TRUE(pair.first == 0 || pair.second == 0);
    }
}

TEST(TestGraph, testRenameVerticesFromEdges) {
    std::vector<std::pair<int, int>> edges = {
        {0, 1}, {3,4}, {9, 10}, {0, 10}
    };
    auto lookup = renameVerticesFromEdges(edges);
    std::vector<int> old_names = { 0, 1, 3, 4, 9, 10 };
    for (int i = 0; i < old_names.size(); i++)
        EXPECT_EQ(lookup[i], old_names[i]);
}

INSTANTIATE_TEST_SUITE_P(
    TestGraph,
    EdgeSubsetGraph,
    ::testing::Values(GraphType::GRID8, GraphType::SUURBALLE)
);
