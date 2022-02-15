#include "fixtures.hh"
#include "pctsp/renaming.hh"

TEST(TestRename, testRenameEdges) {
    std::vector<std::pair<int, int>> old_edges = {
        {0, 1}, {3,4}, {9, 10}, {0, 10}
    };
    boost::bimap<int, int> lookup;
    auto new_edges = renameEdges(lookup, old_edges);
    std::vector<int> old_names = { 0, 1, 3, 4, 9, 10 };
    for (int i = 0; i < old_names.size(); i++)
        EXPECT_EQ(getOldVertex(lookup, i), old_names[i]);
}

TEST_P(BadlyNamedFixture, testGetNewEdges) {
    auto old_edges = getBadlyNamedEdges();
    boost::bimap<PCTSPvertex, PCTSPvertex> lookup;
    auto expected = renameEdges(lookup, old_edges);
    auto actual = getNewEdges(lookup, old_edges);
    EXPECT_THAT(expected, ::testing::ContainerEq(actual));
}

TEST_P(BadlyNamedFixture, testFillCostMapFromRenamedMap) {
    auto old_edges = getBadlyNamedEdges();
    auto old_cost_map = getOldCostMap();
    boost::bimap<PCTSPvertex, PCTSPvertex> lookup;
    auto new_edges = renameEdges(lookup, old_edges);
    PCTSPgraph graph;
    EdgeCostMap new_cost_map = boost::get(edge_weight, graph);
    addEdgesToGraph(graph, new_edges);
    fillCostMapFromRenamedMap(graph, new_cost_map, old_cost_map, lookup);
    for (int i = 0; i < old_edges.size(); i++) {
        auto edge = boost::edge(new_edges[i].first, new_edges[i].second, graph).first;
        EXPECT_EQ(old_cost_map[old_edges[i]], new_cost_map[edge]);
    }
}

TEST_P(BadlyNamedFixture, testFillRenamedVertexMap) {
    auto old_edges = getBadlyNamedEdges();
    auto old_prize = getOldPrizeMap();
    boost::bimap<PCTSPvertex, PCTSPvertex> lookup;
    auto new_edges = renameEdges(lookup, old_edges);
    std::vector<int> new_prize (old_prize.size());
    fillRenamedVertexMap(new_prize, old_prize, lookup);
    for (PCTSPvertex i = 0; i < old_prize.size(); i++) {
        auto old = getOldVertex(lookup, i);
        EXPECT_EQ(old_prize[old], new_prize[i]);
    }
}

TEST(TestRename, testGetOldVertex) {
    boost::bimap<int, int> vertex_id_map;
    typedef boost::bimap<int, int>::value_type position;
    int zero = 0;
    int one = 1;
    int two = 2;
    vertex_id_map.insert(position(0, 1));
    vertex_id_map.insert(position(1, 2));

    EXPECT_EQ(getOldVertex(vertex_id_map, zero), one);
    EXPECT_EQ(getOldVertex(vertex_id_map, one), two);
}

TEST(TestRename, testGetNewVertex) {
    boost::bimap<int, int> vertex_id_map;
    typedef boost::bimap<int, int>::value_type position;
    int zero = 0;
    int one = 1;
    int two = 2;
    vertex_id_map.insert(position(0, 1));
    vertex_id_map.insert(position(1, 2));

    EXPECT_EQ(getNewVertex(vertex_id_map, one), zero);
    EXPECT_EQ(getNewVertex(vertex_id_map, two), one);
}

INSTANTIATE_TEST_SUITE_P(TestRename, BadlyNamedFixture, ::testing::Values(
    BadlyNamedEdges::WELL_NAMED, BadlyNamedEdges::BADLY_NAMED, BadlyNamedEdges::EMPTY, BadlyNamedEdges::REVERSE_NAMED
));