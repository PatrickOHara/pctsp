#ifndef __PCTSP_CYCLE_COVER__
#define __PCTSP_CYCLE_COVER__

#include <boost/graph/graph_traits.hpp>
#include <objscip/objscip.h>

#include "graph.hh"

template <typename TGraph, typename TVertexIt, typename TEdgeVarMap>
SCIP_RETCODE addCycleCover(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    TGraph& graph,
    TVertexIt& first_vertex_it,
    TVertexIt& last_vertex_it,
    TEdgeVarMap& edge_variable_map,
    SCIP_SOL* sol,
    SCIP_RESULT* result,
    std::string& name
) {

    return SCIP_OKAY;
}

SCIP_RETCODE addCycleCover(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    PCTSPgraph& graph,
    std::vector<PCTSPvertex>& vertices_in_cover,
    PCTSPedgeVariableMap& edge_variable_map,
    SCIP_SOL* sol,
    SCIP_RESULT* result,
    std::string& name
);

#endif