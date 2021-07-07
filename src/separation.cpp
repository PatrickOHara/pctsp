#include "pctsp/separation.hh"
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/typeof/typeof.hpp>
#include <iostream>

bool isGraphSimpleCycle(PCTSPgraph& graph, std::vector<int>& component_vector) {
    if (boost::num_vertices(graph) == 0) return false;
    if (boost::num_edges(graph) == 0) return false;
    int n_components = boost::connected_components(graph, &component_vector[0]);
    if (n_components != 1) return false;    // a simple cycle must be connected

    for (auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
        if (boost::degree(vertex, graph) != 2) return false;
    }
    return true;
}


bool isSimpleCycle(PCTSPgraph& graph, std::vector<PCTSPedge>& edge_vector) {
    if (edge_vector.size() == 0) return false;

    std::map<PCTSPvertex, std::list<PCTSPvertex>> vertex_count;
    for (auto const& edge : edge_vector) {
        auto source = boost::source(edge, graph);
        auto target = boost::target(edge, graph);
        if (source == target)
            return false;
        if (vertex_count.find(source) == vertex_count.end()) {
            std::list<PCTSPvertex> adj_list = { target };
            vertex_count.emplace(source, adj_list);
        }
        else
            vertex_count[source].emplace_back(target);

        if (vertex_count.find(target) == vertex_count.end()) {
            std::list<PCTSPvertex> adj_list = { source };
            vertex_count.emplace(target, adj_list);
        }
        else
            vertex_count[target].emplace_back(source);

        if (vertex_count[source].size() > 2 || vertex_count[target].size() > 2)
            return false;
    }

    auto edge = edge_vector[0];
    auto source = boost::source(edge, graph);
    auto current = boost::target(edge, graph);
    auto prev = source;
    int i = 0;
    while (current != source && i < edge_vector.size()) {
        auto adj_list = vertex_count[current];
        if (adj_list.size() != 2)
            return false;
        auto it = adj_list.begin();
        PCTSPvertex next;
        if (*it == prev)
            next = *(++it);
        else
            next = *it;
        prev = current;
        current = next;
        i++;
    }
    return i == edge_vector.size() - 1;
}

const CapacityType FLOW_FLOAT_MULTIPLIER = 1000000;

CapacityVector getCapacityVectorFromSol(
    SCIP* scip,
    PCTSPgraph& graph,
    SCIP_SOL* sol,
    std::map<PCTSPedge, SCIP_VAR*>& edge_variable_map
) {
    CapacityVector capacity;
    auto edges = getSolutionEdges(scip, graph, sol, edge_variable_map);
    for (auto const& edge : edges) {
        SCIP_VAR* var = edge_variable_map[edge];
        double value = (double)SCIPgetSolVal(scip, sol, var);   // value is less than or equal to 2
        CapacityType int_value;
        if (SCIPisZero(scip, value))
            int_value = 0;
        else if (SCIPisZero(scip, value - 1.0))
            int_value = FLOW_FLOAT_MULTIPLIER;
        else if (SCIPisZero(scip, value - 2.0))
            int_value = FLOW_FLOAT_MULTIPLIER * 2;
        else
            int_value = (CapacityType)((double)FLOW_FLOAT_MULTIPLIER * value);
        capacity.push_back(int_value);
    }
    return capacity;
}

// A graphic of the min-cut is available at
// <http://www.boost.org/doc/libs/release/libs/graph/doc/stoer_wagner_imgs/stoer_wagner.cpp.gif>
int runMinCut()
{
    using namespace std;
    // define the 16 edges of the graph. {3, 4} means an undirected edge between
    // vertices 3 and 4.
    VertexPairVector edges = { { 3, 4 }, { 3, 6 }, { 3, 5 }, { 0, 4 }, { 0, 1 },
        { 0, 6 }, { 0, 7 }, { 0, 5 }, { 0, 2 }, { 4, 1 }, { 1, 6 }, { 1, 5 },
        { 6, 7 }, { 7, 5 }, { 5, 2 }, { 3, 4 } };

    // for each of the 16 edges, define the associated edge weight. ws[i] is the
    // weight for the edge that is described by edges[i].
    CapacityType ws[] = { 0, 3, 1, 3, 1, 2, 6, 1, 8, 1, 1, 80, 2, 1, 1, 4 };

    // construct the graph object. 8 is the number of vertices, which are
    // numbered from 0 through 7, and 16 is the number of edges.
    UndirectedCapacityGraph g(&edges[0], &edges[0] + edges.size(), ws, 8, 16);

    // define a property map, `parities`, that will store a boolean value for
    // each vertex. Vertices that have the same parity after
    // `stoer_wagner_min_cut` runs are on the same side of the min-cut.
    BOOST_AUTO(parities,
        boost::make_one_bit_color_map(
            num_vertices(g), get(boost::vertex_index, g)));

    // run the Stoer-Wagner algorithm to obtain the min-cut weight. `parities`
    // is also filled in.
    int w = boost::stoer_wagner_min_cut(
        g, get(boost::edge_weight, g), boost::parity_map(parities));

    cout << "The min-cut weight of G is " << w << ".\n" << endl;
    assert(w == 7);

    cout << "One set of vertices consists of:" << endl;
    size_t i;
    for (i = 0; i < num_vertices(g); ++i)
    {
        if (get(parities, i))
            cout << i << endl;
    }
    cout << endl;

    cout << "The other set of vertices consists of:" << endl;
    for (i = 0; i < num_vertices(g); ++i)
    {
        if (!get(parities, i))
            cout << i << endl;
    }
    cout << endl;

    return EXIT_SUCCESS;
}
