#include "pctsp/pygraph.hh"
#include <gtest/gtest.h>



TEST(TestPyGraph, testGraphFromPyEdgeList) {
    Py_Initialize();
    py::tuple edge = py::make_tuple(0, 1);
    EXPECT_EQ(py::extract<int>(edge[0]), 0);
    py::list edge_list;
    edge_list.append(edge);
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);
    EXPECT_EQ(py::len(edge_list), boost::num_edges(graph));
    EXPECT_TRUE(boost::edge(0, 1, graph).second);
    EXPECT_TRUE(boost::edge(1, 0, graph).second);
}

TEST(TestPyGraph, testPrizeMapFromPyDict) {
    Py_Initialize();
    typedef BoostPyBimap::value_type position;
    py::tuple prize1 = py::make_tuple(0, 1);
    py::tuple prize2 = py::make_tuple(2, 5);
    py::list prize_list;
    prize_list.append(prize1);
    prize_list.append(prize2);
    py::dict prize_dict(prize_list);
    EXPECT_EQ(py::len(prize_list), py::len(prize_list));

    // get the graph and vertex lookup
    py::list edge_list;
    edge_list.append(py::make_tuple(0,2));
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);

    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    fillPrizeMapFromPyDict(prize_map, prize_dict, vertex_id_map);

    EXPECT_EQ(prize_map[0], 1);
    EXPECT_EQ(prize_map[1], 5);
}

TEST(TestPyGraph, testGetCostMapFromPyDict) {
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
    BoostPyBimap vertex_id_map;
    PCTSPgraph graph = graphFromPyEdgeList(edge_list, vertex_id_map);

    // get the std::cost map and check the costs are correct
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    fillCostMapFromPyDict(graph, cost_map, cost_dict, vertex_id_map);
    int one = 1;
    int two = 2;
    int four = 4;
    PCTSPedge boost_edge1 = boost::edge(getNewVertex(vertex_id_map, two),
        getNewVertex(vertex_id_map, four), graph)
        .first;
    PCTSPedge boost_edge2 = boost::edge(getNewVertex(vertex_id_map, one),
        getNewVertex(vertex_id_map, two), graph)
        .first;
    EXPECT_EQ(cost_map[boost_edge1], 5);
    EXPECT_EQ(cost_map[boost_edge2], 7);
}

TEST(TestPyGraph, testGetPyVertexList) {
    std::list<PCTSPvertex> vertex_list = { 0, 1, 2 };
    BoostPyBimap vertex_id_map;
    typedef BoostPyBimap::value_type position;
    PCTSPvertex zero = 0;
    PCTSPvertex one = 1;
    PCTSPvertex two = 2;
    int three = 3;
    int five = 5;
    int seven = 7;
    vertex_id_map.insert(position(zero, seven));
    vertex_id_map.insert(position(one, three));
    vertex_id_map.insert(position(two, five));
    auto py_list = getPyVertexList(vertex_id_map, vertex_list);
    EXPECT_EQ(py_list[0], 7);
    EXPECT_EQ(py_list[1], 3);
    EXPECT_EQ(py_list[2], 5);
}

TEST(TestPyGraph, testGetBoostVertexList) {
    py::list py_list;
    typedef BoostPyBimap::value_type position;
    py_list.append(7);
    py_list.append(3);
    py_list.append(5);
    BoostPyBimap vertex_id_map;
    PCTSPvertex zero = 0;
    PCTSPvertex one = 1;
    PCTSPvertex two = 2;
    int three = 3;
    int five = 5;
    int seven = 7;
    vertex_id_map.insert(position(zero, seven));
    vertex_id_map.insert(position(one, three));
    vertex_id_map.insert(position(two, five));
    auto vertex_list = getBoostVertexList(vertex_id_map, py_list);
    auto it = vertex_list.begin();
    EXPECT_EQ(*(it++), 0);
    EXPECT_EQ(*(it++), 1);
    EXPECT_EQ(*(it), 2);
}

TEST(TestPyGraph, testGetPyEdgeList) {
    PCTSPgraph graph;
    PCTSPedge edge1 = boost::add_edge(0, 1, graph).first;
    PCTSPedge edge2 = boost::add_edge(1, 2, graph).first;
    typedef BoostPyBimap::value_type position;
    BoostPyBimap vertex_id_map;
    PCTSPvertex zero = 0;
    PCTSPvertex one = 1;
    PCTSPvertex two = 2;
    int three = 3;
    int five = 5;
    int seven = 7;
    vertex_id_map.insert(position(zero, seven));
    vertex_id_map.insert(position(one, three));
    vertex_id_map.insert(position(two, five));

    std::list<PCTSPedge> edge_list = { edge1, edge2 };
    py::list py_edge_list = getPyEdgeList(graph, vertex_id_map, edge_list);    EXPECT_EQ(py::len(py_edge_list), edge_list.size());
    EXPECT_TRUE(py_edge_list.contains(py::make_tuple(7, 3)));
}
