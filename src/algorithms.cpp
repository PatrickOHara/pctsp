/** Algorithms for the prize collecting TSP */

#include "pctsp/algorithms.hh"

SCIP_RETCODE addHeuristicVarsToSolver(
    SCIP* scip,
    SCIP_HEUR* heur,
    std::vector<SCIP_VAR*>& vars
) {
    SCIP_SOL* sol;
    SCIP_CALL(SCIPcreateSol(scip, &sol, heur));
    for (SCIP_VAR* var : vars) {
        SCIP_CALL(SCIPsetSolVal(scip, sol, var, 1.0));
    }
    SCIP_Bool success;
    SCIP_RESULT* result;
    SCIP_CALL(SCIPtrySol(scip, sol, FALSE, FALSE, FALSE, FALSE, FALSE, &success));

    if (success)
        *result = SCIP_FOUNDSOL;
    else
        *result = SCIP_DIDNOTFIND;

    SCIP_CALL(SCIPfreeSol(scip, &sol));

    return SCIP_OKAY;
}
