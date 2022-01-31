
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
  std::list<PCTSPvertex> getSmallTour();
};

template <typename It>
void expectEqualContainers(
  It& first_container_begin,
  It& first_container_end,
  It& second_container_begin,
  It& second_container_end
) {
    while ((first_container_begin != first_container_end) & (second_container_begin != second_container_end)) {
        EXPECT_EQ(*first_container_begin, *second_container_begin);
        first_container_begin ++;
        second_container_begin ++;
    }
}

template <typename T>
void expectEqualLists(std::list<T>& first, std::list<T>& second) {
  auto first_container_begin = first.begin();
  auto first_container_end = first.end();
  auto second_container_begin = second.begin();
  auto second_container_end = second.end();
  expectEqualContainers(first_container_begin, first_container_end, second_container_begin, second_container_end);
}

template <typename T>
void expectEqualVectors(std::vector<T>& first, std::vector<T>& second) {
  auto first_container_begin = first.begin();
  auto first_container_end = first.end();
  auto second_container_begin = second.begin();
  auto second_container_end = second.end();
  expectEqualContainers(first_container_begin, first_container_end, second_container_begin, second_container_end);
}
