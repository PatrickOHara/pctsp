
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
  EdgeCostMap getCostMap(PCTSPgraph& graph);
  VertexPrizeMap getPrizeMap(PCTSPgraph& graph);
  VertexPrizeMap getGenOnePrizeMap(PCTSPgraph& graph);
  std::vector<std::pair<int, int>> getEdgeVector();
  std::vector<int> getVertexVector();
  int getNumVertices();
  int getQuota();
  PCTSPvertex getRootVertex();
  std::string getParamName();
};
