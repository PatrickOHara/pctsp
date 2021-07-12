#include "fixtures.hh"
#include "pctsp/renaming.hh"
#include <gtest/gtest.h>

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