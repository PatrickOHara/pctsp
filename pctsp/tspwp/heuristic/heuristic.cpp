
#include <boost/graph/graph_traits.hpp>
#include <boost/python.hpp>
#include <graph_tool.hh>
#include <iostream>

#include "pctsp/heuristic.hh"
#include "pctsp/pyutils.hh"

namespace gt = graph_tool;
namespace py = boost::python;

float unitary_gain(int prize_v, int cost_uw, int cost_uv, int cost_vw) {
    // v is the new vertex to insert (denoted j by Dell'Amico et al. [1998])
    // u and w already exist in the cycle (denoted h_i and h_{i+1} by Dell'Amico
    // et al. [1998])) cast to float to avoid rounding
    return (float)prize_v / (float)(cost_uv + cost_vw - cost_uw);
}

py::list collapse_bind(gt::GraphInterface &gi, py::list py_tour,
                       boost::any cost_map, boost::any prize_map, int quota,
                       int root_vertex) {
    typedef typename std::list<int> tour_type;
    // convert python list to cpp queue
    tour_type tour = pyListToStdList<int>(py_tour);

    typedef typename gt::vprop_map_t<int32_t>::type vprop_t;
    auto prize_int_property_unchecked =
        boost::any_cast<vprop_t>(prize_map).get_unchecked();
    typedef typename gt::eprop_map_t<int32_t>::type eprop_t;
    auto cost_int_property_unchecked =
        boost::any_cast<eprop_t>(cost_map).get_unchecked();

    std::list<int> new_tour;

    // use graph tool dispatch to get python graph into boost graph
    gt::gt_dispatch<>()(
        [&](auto &g) {
            new_tour =
                collapse(g, tour, cost_int_property_unchecked,
                         prize_int_property_unchecked, quota, root_vertex);
        },
        gt::never_directed())(gi.get_graph_view());
    // convert back to python list
    return stdListToPyList<int>(new_tour);
};

py::list extend_bind(gt::GraphInterface &gi, py::list py_tour,
                     boost::any cost_map, boost::any prize_map) {
    typedef typename std::list<int> tour_type;
    // convert python list to cpp queue
    tour_type tour = pyListToStdList<int>(py_tour);

    typedef typename gt::vprop_map_t<int32_t>::type vprop_t;
    auto prize_int_property_unchecked =
        boost::any_cast<vprop_t>(prize_map).get_unchecked();
    typedef typename gt::eprop_map_t<int32_t>::type eprop_t;
    auto cost_int_property_unchecked =
        boost::any_cast<eprop_t>(cost_map).get_unchecked();
    // use graph tool dispatch to get python graph into boost graph
    gt::gt_dispatch<>()(
        [&](auto &g) {
            extend(g, tour, cost_int_property_unchecked,
                   prize_int_property_unchecked);
        },
        gt::never_directed())(gi.get_graph_view());
    // convert back to python list
    return stdListToPyList<int>(tour);
};

py::list extend_until_prize_feasible_bind(gt::GraphInterface &gi,
                                          py::list py_tour, boost::any cost_map,
                                          boost::any prize_map, int quota) {
    typedef typename std::list<int> tour_type;
    // convert python list to cpp queue
    tour_type tour = pyListToStdList<int>(py_tour);

    typedef typename gt::vprop_map_t<int32_t>::type vprop_t;
    auto prize_int_property_unchecked =
        boost::any_cast<vprop_t>(prize_map).get_unchecked();
    typedef typename gt::eprop_map_t<int32_t>::type eprop_t;
    auto cost_int_property_unchecked =
        boost::any_cast<eprop_t>(cost_map).get_unchecked();
    // use graph tool dispatch to get python graph into boost graph
    gt::gt_dispatch<>()(
        [&](auto &g) {
            extend_until_prize_feasible(g, tour, cost_int_property_unchecked,
                                        prize_int_property_unchecked, quota);
        },
        gt::never_directed())(gi.get_graph_view());
    // convert back to python list
    return stdListToPyList<int>(tour);
};

BOOST_PYTHON_MODULE(libheuristic) {
    using namespace py;
    def("collapse_bind", collapse_bind);
    def("extend_bind", extend_bind);
    def("extend_until_prize_feasible_bind", extend_until_prize_feasible_bind);
    def("unitary_gain", unitary_gain);
}