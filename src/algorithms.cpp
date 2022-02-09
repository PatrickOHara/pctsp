/** Algorithms for the prize collecting TSP */

#include "pctsp/algorithms.hh"

SCIP_RETCODE addHeuristicVarsToSolver(
    SCIP* scip,
    SCIP_HEUR* heur,
    std::vector<SCIP_VAR*> vars
) {
    SCIP_SOL* sol;
    SCIP_CALL(SCIPcreateSol(scip, &sol, heur));
    for (SCIP_VAR* var : vars) {
        SCIP_CALL(SCIPsetSolVal(scip, sol, var, 1.0));
    }
    SCIP_Bool success;
    SCIP_RESULT* result;
    SCIP_CALL(SCIPaddSolFree(scip, &sol, &success));

    // if (success)
    //     *result = SCIP_FOUNDSOL;
    // else
    //     *result = SCIP_DIDNOTFIND;
    return SCIP_OKAY;
}

SCIP_RETCODE modelPrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<PCTSPedge>& solution_edges,
    EdgeCostMap& cost_map,
    VertexPrizeMap& prize_map,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex
) {
    // add self loops to graph - we assume the input graph is simple
    if (hasSelfLoopsOnAllVertices(graph) == false) {
        addSelfLoopsToGraph(graph);
        assignZeroCostToSelfLoops(graph, cost_map);
    }

    std::map<PCTSPedge, SCIP_VAR*> edge_variable_map;
    std::map<PCTSPedge, PrizeNumberType> weight_map;
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);
    std::vector<NodeStats> node_stats;  // save node statistics
    ProbDataPCTSP* probdata = new ProbDataPCTSP(&graph, &root_vertex, &edge_variable_map, &quota, &node_stats);

    // initialise empty model
    const char* name = "modelPrizeCollectingTSP";
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));
    SCIP_CALL(SCIPcreateObjProb(scip, name, probdata, true));

    SCIP_CALL(PCTSPmodelWithoutSECs(scip, graph, cost_map, weight_map, quota, root_vertex, edge_variable_map));
    SCIP_CALL(SCIPincludeObjConshdlr(scip, new PCTSPconshdlrSubtour(scip, true, 1, true, 1), TRUE));

    // turn off presolving
    SCIPsetIntParam(scip, "presolving/maxrounds", 0);

    // add the subtour elimination constraints as cutting planes
    SCIP_CONS* cons;
    std::string cons_name("subtour-constraint");
    PCTSPcreateBasicConsSubtour(scip, &cons, cons_name, graph, root_vertex);
    SCIPaddCons(scip, cons);
    SCIPreleaseCons(scip, &cons);

    // add selected heuristics to reduce the upper bound on the optimal
    if (solution_edges.size() > 0) {
        auto first = solution_edges.begin();
        auto last = solution_edges.end();
        BOOST_LOG_TRIVIAL(info) << "Adding starting solution to solver.";
        SCIP_HEUR* heur = NULL;
        addHeuristicEdgesToSolver(scip, graph, heur, edge_variable_map, first, last);
    }
    SCIPsolve(scip);
    return SCIP_OKAY;
}

SCIP_RETCODE modelPrizeCollectingTSP(
    SCIP* scip,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& solution_edges,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex
) {
    // add edges to empty graph
    PCTSPgraph graph;
    auto start = edge_list.begin();
    auto end = edge_list.end();
    addEdgesToGraph(graph, start, end);

    // get the prize and costs
    VertexPrizeMap prize_map = boost::get(vertex_distance, graph);
    EdgeCostMap cost_map = boost::get(edge_weight, graph);
    for (auto const& [vertex, prize] : prize_dict) {
        prize_map[vertex] = prize;
    }
    for (auto const& [edge_pair, cost] : cost_dict) {
        auto edge = boost::edge(edge_pair.first, edge_pair.second, graph);
        if (! edge.second) throw EdgeNotFoundException(edge_pair.first, edge_pair.second);
        cost_map[edge.first] = cost;
    }
    // initialise the heuristic solution
    std::vector<PCTSPedge> solution;
    if (solution_edges.size() > 0) {
        auto pairs_first = solution_edges.begin();
        auto pairs_last = solution_edges.end();
        solution = edgesFromVertexPairs(graph, pairs_first, pairs_last);
    }

    return modelPrizeCollectingTSP(scip, graph, solution, cost_map, prize_map, quota, root_vertex);
}
