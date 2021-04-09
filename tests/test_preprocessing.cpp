/** Tests for preprocessing algorithms */

#include "fixtures.hh"
#include "pctsp/preprocessing.hh"
#include <boost/graph/copy.hpp>

TEST_F(SuurballeGraphFixture, testAddSelfLoopsToGraph) {
    // strangely the degree of a self loop vertex is two?
    PCTSPgraph simple_graph = get_suurballe_graph();
    PCTSPgraph graph;
    boost::copy_graph(simple_graph, graph);
    auto cost_map = get(&PCTSPedgeProperties::cost, simple_graph);
    EXPECT_FALSE(hasSelfLoopsOnAllVertices(simple_graph));
    addSelfLoopsToGraph(graph);
    EXPECT_TRUE(hasSelfLoopsOnAllVertices(graph));
    for (auto vertex : boost::make_iterator_range(vertices(graph))) {
        EXPECT_EQ(boost::degree(vertex, graph) - 2,
                  boost::degree(vertex, simple_graph));
        auto edge = boost::edge(vertex, vertex, graph);
        EXPECT_TRUE(edge.second);
    }
}

/** test the prize is moved onto weights on the edges */
TEST_F(SuurballeGraphFixture, testPutPrizeOntoEdgeWeights) {
    PCTSPgraph graph = get_suurballe_graph();
    auto prize_map = get(&PCTSPvertexProperties::prize, graph);

    int total_prize = 0;
    int i = 0;
    for (auto vertex : boost::make_iterator_range(vertices(graph))) {
        total_prize += prize_map[vertex];
        boost::add_edge(vertex, vertex, {0}, graph);
        i += 1;
    }

    // setup maps
    typedef typename graph_traits<PCTSPgraph>::edge_descriptor Edge;
    std::map<Edge, int> weight_map;

    // move the prize onto the edge weights
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);

    // for all edges that are self loops, the weight is the
    // same as the prize of the vertex
    // for all edges that are not self loops, the weight is zero
    // so the sum of the edge weights is equal to the sum of the prizes
    int total_weight = 0;
    for (auto it = weight_map.begin(); it != weight_map.end(); it++) {
        total_weight += it->second;
    }
    EXPECT_EQ(total_prize, boost::num_vertices(graph));
    EXPECT_EQ(total_prize, total_weight);
}