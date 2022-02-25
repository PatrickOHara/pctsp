
#include "pctsp/graph.hh"
#include <boost/graph/adjacency_list.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>


using namespace boost;

enum class GraphType {
	COMPLETE4,
	COMPLETE5,
	COMPLETE25,
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

enum class BadlyNamedEdges {
	WELL_NAMED,
	BADLY_NAMED,
	EMPTY,
	REVERSE_NAMED,
};

class BadlyNamedFixture : public::testing::TestWithParam<BadlyNamedEdges> {
	public:
		std::vector<std::pair<PCTSPvertex, PCTSPvertex>> getBadlyNamedEdges();
		std::map<std::pair<PCTSPvertex, PCTSPvertex>, int> getOldCostMap();
		std::map<PCTSPvertex, int> getOldPrizeMap();
};
