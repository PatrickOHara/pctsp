/** Algorithms for the prize collecting TSP */

#include "pctsp/algorithms.hh"
#include "pctsp/node_selection.hh"
#include "pctsp/separation.hh"

struct SCIP_ProbData {
   ProbDataPCTSP*    objprobdata;        /**< user problem data object */
   SCIP_Bool             deleteobject;       /**< should the user problem data object be deleted when problem is freed? */
};

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
    SCIP_CALL(SCIPaddSolFree(scip, &sol, &success));

    return SCIP_OKAY;
}

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> solvePrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<PCTSPedge>& heuristic_edges,
    EdgeCostMap& cost_map,
    VertexPrizeMap& prize_map,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name
) {
    auto edge_var_map = modelPrizeCollectingTSP(scip, graph, heuristic_edges, cost_map, prize_map, quota, root_vertex, name);
    SCIPsolve(scip);
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    auto solution_edges = getSolutionEdges(scip, graph, sol, edge_var_map);
    return getVertexPairVectorFromEdgeSubset(graph, solution_edges);
}

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> solvePrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& heuristic_edges,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name
) {
    auto edge_var_map = modelPrizeCollectingTSP(scip, graph, edge_list, heuristic_edges, cost_dict, prize_dict, quota, root_vertex, name);
    SCIPsolve(scip);
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    auto solution_edges = getSolutionEdges(scip, graph, sol, edge_var_map);
    return getVertexPairVectorFromEdgeSubset(graph, solution_edges);
}

SummaryStats getSummaryStatsFromSCIP(SCIP* scip) {
    // get the node event handler
    auto objeventhdlr = SCIPfindObjEventhdlr(scip, NODE_EVENTHDLR_NAME.c_str());
    unsigned int n_disjoint_sec = 0;
    unsigned int n_flow_sec = 0;
    if (objeventhdlr != 0) {
        NodeEventhdlr* node_eventhdlr =  dynamic_cast<NodeEventhdlr*>(objeventhdlr);
        auto node_stats = node_eventhdlr->getNodeStatsVector();
        n_disjoint_sec = numDisjointTourSECs(node_stats);
        n_flow_sec = numMaxflowMincutSECs(node_stats);
    }

    // get cost cover event handlers
    auto spcc = SCIPfindObjEventhdlr(scip, SHORTEST_PATH_COST_COVER_NAME.c_str());
    unsigned int num_cost_cover_shortest_paths = 0;
    if (spcc != 0) {
        CostCoverEventHandler* shortest_path_cc_hdlr = dynamic_cast<CostCoverEventHandler*>(spcc);
        num_cost_cover_shortest_paths = shortest_path_cc_hdlr->getNumConssAdded();
    }
    unsigned int num_cost_cover_disjoint_paths = 0;
    auto dpcc = SCIPfindObjEventhdlr(scip, DISJOINT_PATHS_COST_COVER_NAME.c_str());
    if (dpcc != 0) {
        CostCoverEventHandler* disjoint_paths_cc_hdlr = dynamic_cast<CostCoverEventHandler*>(dpcc);
        num_cost_cover_disjoint_paths = disjoint_paths_cc_hdlr->getNumConssAdded();
    }
    unsigned int num_cycle_cover = getNumCycleCoverCutsAdded(scip);

    return getSummaryStatsFromSCIP(
        scip,
        num_cost_cover_disjoint_paths,
        num_cost_cover_shortest_paths,
        num_cycle_cover,
        n_disjoint_sec,
        n_flow_sec
    );
}

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> solvePrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<PCTSPedge>& heuristic_edges,
    EdgeCostMap& cost_map,
    VertexPrizeMap& prize_map,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    int branching_max_depth,
    unsigned int branching_strategy,
    bool cost_cover_disjoint_paths,
    bool cost_cover_shortest_path,
    bool cycle_cover,
    std::vector<int> disjoint_paths_distances,
    std::string name,
    bool sec_disjoint_tour,
    double sec_lp_gap_improvement_threshold,
    bool sec_maxflow_mincut,
    int sec_max_tailing_off_iterations,
    int sec_sepafreq,
    std::filesystem::path solver_dir,
    float time_limit
) {
    // build filepaths
    std::filesystem::create_directory(solver_dir);
    std::filesystem::path scip_bounds_csv = solver_dir / SCIP_BOUNDS_CSV;
    std::filesystem::path scip_node_stats_csv = solver_dir / SCIP_NODE_STATS_CSV;
    std::filesystem::path scip_logs_txt = solver_dir / SCIP_LOGS_TXT;
    std::filesystem::path pctsp_summary_stats_yaml = solver_dir / PCTSP_SUMMARY_STATS_YAML;

    // add custom message handler
    SCIP_MESSAGEHDLR* handler;
    SCIPcreateMessagehdlrDefault(&handler, false, scip_logs_txt.c_str(), true);
    SCIPsetMessagehdlr(scip, handler);

    // add variables, constraints and the SEC cutting plane
    auto edge_var_map = modelPrizeCollectingTSP(scip, graph, heuristic_edges, cost_map, prize_map, quota, root_vertex, name, sec_disjoint_tour, sec_lp_gap_improvement_threshold, sec_maxflow_mincut, sec_max_tailing_off_iterations, sec_sepafreq);

    // add the cost cover inequalities when a new solution is found
    if (cost_cover_disjoint_paths) {
        includeDisjointPathsCostCover(scip, disjoint_paths_distances);
    }
    if (cost_cover_shortest_path) {
        includeShortestPathCostCover(scip, graph, cost_map, root_vertex);
    }
    // add cycle cover constraint
    auto cycle_cover_conshdlr = new CycleCoverConshdlr(scip);
    if (cycle_cover) {
        SCIPincludeObjConshdlr(scip, cycle_cover_conshdlr, true);
        SCIP_CONS* cycle_cover_cons;
        createBasicCycleCoverCons(scip, &cycle_cover_cons);
        SCIPaddCons(scip, cycle_cover_cons);
        SCIPreleaseCons(scip, &cycle_cover_cons);
    }
    // add event handlers
    NodeEventhdlr* node_eventhdlr = new NodeEventhdlr(scip);
    SCIPincludeObjEventhdlr(scip, node_eventhdlr, TRUE);
    BoundsEventHandler* bounds_handler = new BoundsEventHandler(scip);
    SCIPincludeObjEventhdlr(scip, bounds_handler, TRUE);

    // set branching scheme
    setBranchingStrategy(scip, branching_strategy, branching_max_depth);

    // time limit
    SCIPsetRealParam(scip, "limits/time", time_limit);

    // set seed to obtain reproducible randomness
    SCIPinitializeRandomSeed(scip, PCTSP_DEFAULT_SEED);
    setBranchingRandomSeeds(scip);

    // solve the model
    SCIPsolve(scip);

    // get the solution
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    auto solution_edges = getSolutionEdges(scip, graph, sol, edge_var_map);

    // get the node stats of the solver
    auto node_stats = node_eventhdlr->getNodeStatsVector();
    writeNodeStatsToCSV(node_stats, scip_node_stats_csv);

    // get the summary statistics and write then to file
    auto summary = getSummaryStatsFromSCIP(scip);
    writeNodeStatsToCSV(node_stats, scip_node_stats_csv);
    writeSummaryStatsToYaml(summary, pctsp_summary_stats_yaml);

    // write the logs to txt
    FILE* log_file = fopen(scip_logs_txt.c_str(), "w");
    SCIPprintStatistics(scip, log_file);

    // write the bounds CSV
    std::vector<Bounds> bounds_vector = bounds_handler->getBoundsVector();
    writeBoundsToCSV(bounds_vector, scip_bounds_csv);

    // release any variable we no longer need
    SCIPmessagehdlrRelease(&handler);

    return getVertexPairVectorFromEdgeSubset(graph, solution_edges);
}

std::map<PCTSPedge, SCIP_VAR*> modelPrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<PCTSPedge>& heuristic_edges,
    EdgeCostMap& cost_map,
    VertexPrizeMap& prize_map,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name,
    bool sec_disjoint_tour,
    double sec_lp_gap_improvement_threshold,
    bool sec_maxflow_mincut,
    int sec_max_tailing_off_iterations,
    int sec_sepafreq
) {
    // initialise empty model
    // SCIPincludeDefaultPlugins(scip);

    SCIPincludeDialogDefaultBasic(scip);

    // include branching rules
    std::cout << "Including branching rules." << std::endl;
    includeBranchRules(scip);

    // include constraint handlers
    std::cout << "Including constraint handlers." << std::endl;
    SCIPincludeConshdlrLinear(scip);
    SCIPincludeConshdlrIntegral(scip);
    SCIPincludeConshdlrKnapsack(scip);
    SCIPincludeConshdlrVarbound(scip);

    // node selection rules
    std::cout << "Including node selection rules." << std::endl;
    includeNodeSelection(scip);

    // include separation methods
    std::cout << "Including separation methods." << std::endl;
    includeSeparation(scip);

    // include heuristics
    std::cout << "Including heuristic methods." << std::endl;
    includeHeuristics(scip);

    // add self loops to graph - we assume the input graph is simple
    if (hasSelfLoopsOnAllVertices(graph) == false) {
        addSelfLoopsToGraph(graph);
        assignZeroCostToSelfLoops(graph, cost_map);
    }

    std::map<PCTSPedge, SCIP_VAR*> edge_variable_map;
    std::map<PCTSPedge, PrizeNumberType> weight_map;
    putPrizeOntoEdgeWeights(graph, prize_map, weight_map);
    ProbDataPCTSP* objprobdata = new ProbDataPCTSP(&graph, &root_vertex, &edge_variable_map, &quota);

    std::cout << "Creating problem with object data." << std::endl;
    SCIPcreateObjProb(scip, name.c_str(), objprobdata, true);

    PCTSPmodelWithoutSECs(scip, graph, cost_map, weight_map, quota, root_vertex, edge_variable_map);
    auto conshdlr = new PCTSPconshdlrSubtour(
        scip,
        sec_disjoint_tour,
        sec_lp_gap_improvement_threshold,
        sec_maxflow_mincut,
        sec_max_tailing_off_iterations,
        sec_sepafreq
    );
    SCIPincludeObjConshdlr(scip, conshdlr, TRUE);

    // turn off presolving
    SCIPsetIntParam(scip, "presolving/maxrounds", 0);

    // add the subtour elimination constraints as cutting planes
    SCIP_CONS* cons;
    std::string cons_name("subtour-constraint");
    PCTSPcreateBasicConsSubtour(scip, &cons, cons_name, graph, root_vertex);
    SCIPaddCons(scip, cons);
    SCIPreleaseCons(scip, &cons);

    // add selected heuristics to reduce the upper bound on the optimal
    if (heuristic_edges.size() > 0) {
        auto first = heuristic_edges.begin();
        auto last = heuristic_edges.end();
        BOOST_LOG_TRIVIAL(info) << "Adding starting solution with " << heuristic_edges.size() << " edges to solver.";
        SCIP_HEUR* heur = NULL;
        addHeuristicEdgesToSolver(scip, graph, heur, edge_variable_map, first, last);
    }
    return edge_variable_map;
}

std::map<PCTSPedge, SCIP_VAR*> modelPrizeCollectingTSP(
    SCIP* scip,
    PCTSPgraph& graph,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& edge_list,
    std::vector<std::pair<PCTSPvertex, PCTSPvertex>>& heuristic_edges,
    std::map<std::pair<PCTSPvertex, PCTSPvertex>, CostNumberType>& cost_dict,
    std::map<PCTSPvertex, PrizeNumberType>& prize_dict,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name,
    bool sec_disjoint_tour,
    double sec_lp_gap_improvement_threshold,
    bool sec_maxflow_mincut,
    int sec_max_tailing_off_iterations,
    int sec_sepafreq
) {
    // add edges to empty graph
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
    if (heuristic_edges.size() > 0) {
        auto pairs_first = heuristic_edges.begin();
        auto pairs_last = heuristic_edges.end();
        solution = edgesFromVertexPairs(graph, pairs_first, pairs_last);
    }

    return modelPrizeCollectingTSP(scip, graph, solution, cost_map, prize_map, quota, root_vertex, name, sec_disjoint_tour, sec_maxflow_mincut);
}
