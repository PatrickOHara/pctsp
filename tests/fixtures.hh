
#include "pctsp/graph.hh"
#include <boost/graph/adjacency_list.hpp>
#include <gtest/gtest.h>

using namespace boost;

enum class GraphType {
  COMPLETE4,
  COMPLETE5,
  GRID8,
  SUURBALLE,
};

class GraphFixture : public::testing::TestWithParam<GraphType> {
public:
  PCTSPgraph getGraph();
  PCTSPcostMap getCostMap(PCTSPgraph& graph);
  PCTSPprizeMap getPrizeMap(PCTSPgraph& graph);
  std::vector<std::pair<int, int>> getEdgeVector();
  std::vector<int> getVertexVector();
  int getNumVertices();
  int getQuota();
};

class CompleteGraphParameterizedFixture : public ::testing::TestWithParam<int> {
public:
  PCTSPgraph get_complete_PCTSPgraph();

private:
  void add_vertices_with_prizes(PCTSPgraph& g, int N);
  void add_edges_with_costs(PCTSPgraph& g);
};

class SuurballeGraphFixture : public ::testing::Test {
public:
  PCTSPgraph get_suurballe_graph();
};
