/** Distributionally Robust Algorithms */

#include <assert.h>
#include <scip/cons_nonlinear.h>
#include <scip/expr_sum.h>

#include "pctsp/robust.hh"

void addDistRobustCons(
    SCIP * scip,
    PCTSPgraph& graph,
    EdgeCostMap& costSigmaMap,
    PCTSPedgeVariableMap& edgeVarMap
) {
    // initialize basic variables
    int numEdges = boost::num_edges(graph);
    int numVertices = boost::num_vertices(graph);

    // setup the robust variance constraint
    SCIP_CONS* robustCons;
    double alpha = 1.0;
    int nLhsExprs = numEdges + 1;
    std::vector<double> lhsCoefs (nLhsExprs);
    std::vector<SCIP_EXPR*> lhsExprs (nLhsExprs);

    // iterate over each variable and make it into an expression
    int i = 0;
    for (auto const& [edge, var] : edgeVarMap) {
        lhsCoefs[i] = sqrt(costSigmaMap[edge]); // sigma is coef for x variables
        SCIP_EXPR* expr;
        SCIP_VAR* myVar = var;
        SCIPcreateExprVar(scip, &expr, myVar, NULL, NULL);
        lhsExprs[i] = expr;
        i++;
    }
    assert (i == nLhsExprs - 1);
    // auxilary variables
    SCIP_VAR* t = NULL;
    SCIP_VAR* z = NULL;
    SCIPcreateVarBasic(scip, &t, "t", 0, SCIPinfinity(scip), 1, SCIP_Vartype::SCIP_VARTYPE_CONTINUOUS);
    SCIPcreateVarBasic(scip, &z, "z", 0, SCIPinfinity(scip), 1, SCIP_Vartype::SCIP_VARTYPE_CONTINUOUS);
    SCIPaddVar(scip, t);
    SCIPaddVar(scip, z);
    SCIP_EXPR* tExpr;
    SCIP_EXPR* zExpr;
    SCIPcreateExprVar(scip, &tExpr, t, NULL, NULL);
    SCIPcreateExprVar(scip, &zExpr, z, NULL, NULL);

    // create expression for last item in vector of the LHS
    SCIP_EXPR* lastExpr;
    SCIP_EXPR* sum[2];
    sum[0] = tExpr;
    sum[1] = zExpr;
    std::vector<double> lastCoefs = {1, 1};
    SCIPcreateExprSum(scip, &lastExpr, 2, sum, lastCoefs.data(), - 2 * alpha, NULL, NULL);
    lhsCoefs[nLhsExprs - 1] = 1;
    lhsExprs[nLhsExprs - 1] = lastExpr;

    // set RHS: z + 2a - t
    std::vector<double> rhsCoefs = {1, -1};
    std::vector<SCIP_VAR*> rhsVars = {z, t};
    double rhsOffset = 2 * alpha;   // constant on RHS

    // create the second-order cone constraint
    SCIPcreateConsSOC(
        scip,
        &robustCons,
        "robust-variance-soc",
        nLhsExprs,
        lhsExprs.data(),
        lhsCoefs.data(),
        NULL,
        0,
        rhsVars.size(),
        rhsVars.data(),
        rhsCoefs.data(),
        rhsOffset
    );
    // add the second-order cone constraint
    SCIPaddCons(scip, robustCons);
}

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> solveDistRobustPrizeCollectingTsp(
    SCIP * scip,
    PCTSPgraph& graph,
    EdgeCostMap& costMeanMap,
    EdgeCostMap& costSigmaMap,
    VertexPrizeMap& prizeMap,
    PrizeNumberType& quota,
    PCTSPvertex& rootVertex,
    std::string& name
) {
    // add variables, constraints and the SEC cutting plane
    std::vector<PCTSPedge> heuristicEdges = {};
    PCTSPedgeVariableMap edgeVarMap = modelPrizeCollectingTSP(
        scip, graph, heuristicEdges, costMeanMap, prizeMap, quota, rootVertex, name);

    // add distributionally robust constraints
    addDistRobustCons(scip, graph, costSigmaMap, edgeVarMap);

    // solve the model
    SCIPsolve(scip);

    // get the solution
    std::vector<PCTSPedge> solutionEdges = std::vector<PCTSPedge>();
    if (SCIPgetNSols(scip) > 0) {
        SCIP_SOL* sol = SCIPgetBestSol(scip);
        solutionEdges = getSolutionEdges(scip, graph, sol, edgeVarMap);
    }
    // return solution
    return getVertexPairVectorFromEdgeSubset(graph, solutionEdges);
}

/** creates and captures a nonlinear constraint that is a second-order cone constraint with all its constraint flags set to their default values
 *
 * \f$\sqrt{\gamma + \sum_{i=1}^{n} (\alpha_i\, (x_i + \beta_i))^2} \leq \alpha_{n+1}\, (x_{n+1}+\beta_{n+1})\f$
 *
 *  @note the constraint gets captured, hence at one point you have to release it using the method SCIPreleaseCons()
 */
SCIP_RETCODE SCIPcreateConsSOC(
    SCIP*                 scip,               /**< SCIP data structure */
    SCIP_CONS**           cons,               /**< pointer to hold the created constraint */
    const char*           name,               /**< name of constraint */
    int                   nlhsexprs,          /**< number of variables on left hand side of constraint (n) */
    SCIP_EXPR**           lhsexprs,           /**< array of expressions on left hand side (x_i) */
    SCIP_Real*            lhscoefs,           /**< array with coefficients of left hand side variables (alpha_i), or NULL if all 1.0 */
    SCIP_Real*            offsets,            /**< array with offsets of variables (beta_i), or NULL if all 0.0 */
    SCIP_Real             constant,           /**< constant on left hand side (gamma) */
    int                   nrhsvars,           /**< number of variables on right hand side of constraint */
    SCIP_VAR**            rhsvars,            /**< variables on right hand side of constraint */
    SCIP_Real*            rhscoeffs,          /**< coefficients of variables on right hand side */
    SCIP_Real             rhsoffset           /**< offset of variable on right hand side (beta_{n+1}) */
)
{
    SCIP_EXPR* expr;
    SCIP_EXPR* lhssum;
    SCIP_EXPR* terms[2];
    SCIP_Real termcoefs[2];
    int i;

    assert(lhsexprs != NULL || nlhsexprs == 0);

    SCIP_CALL( SCIPcreateExprSum(scip, &lhssum, 0, NULL, NULL, constant, NULL, NULL) );  /* gamma */
    for( i = 0; i < nlhsexprs; i++ )
    {
        SCIP_EXPR* powexpr;
        SCIP_EXPR* term;

        if( offsets != NULL && offsets[i] != 0.0 )
        {
            SCIP_EXPR* sum;
            SCIP_CALL( SCIPcreateExprSum(scip, &sum, 1, &lhsexprs[i], NULL, offsets[i], NULL, NULL) );  /* x_i + beta_i */
            SCIP_CALL( SCIPcreateExprPow(scip, &powexpr, sum, 2.0, NULL, NULL) );   /* (x_i + beta_i)^2 */
            SCIP_CALL( SCIPreleaseExpr(scip, &sum) );
        }
        else
        {
            // FIXME looks like too many edges, errors at i=20
            term = lhsexprs[i];
            SCIP_CALL( SCIPcreateExprPow(scip, &powexpr, term, 2.0, NULL, NULL) );  /* x_i^2 */
        }
        SCIP_CALL(SCIPreleaseExpr(scip, &term));
        SCIP_CALL( SCIPappendExprSumExpr(scip, lhssum, powexpr, lhscoefs != NULL ? lhscoefs[i]*lhscoefs[i] : 1.0) );  /* + alpha_i^2 (x_i + beta_i)^2 */
        SCIP_CALL( SCIPreleaseExpr(scip, &powexpr) );
    }

    SCIP_CALL( SCIPcreateExprPow(scip, &terms[0], lhssum, 0.5, NULL, NULL) );  /* sqrt(...) */
    SCIP_CALL( SCIPreleaseExpr(scip, &lhssum) );
    termcoefs[0] = 1.0;

    //    SCIP_CALL( SCIPcreateExprVar(scip, &terms[1], rhsvar, NULL, NULL) );  /* x_{n+1} */
    SCIP_EXPR* rhssum[nrhsvars];
    for (i = 0; i < nrhsvars; ++i) {
        SCIP_CALL(SCIPcreateExprVar(scip, &rhssum[i], rhsvars[i], NULL, NULL));
    }
    SCIP_CALL( SCIPcreateExprSum(scip, &terms[1], nrhsvars, rhssum, rhscoeffs, 0.0, NULL, NULL));
    termcoefs[1] = -1.0;

    SCIP_CALL( SCIPcreateExprSum(scip, &expr, 2, terms, termcoefs, 0.0, NULL, NULL) );  /* sqrt(...) - alpha_{n+1}x_{n_1} */

    SCIP_CALL( SCIPreleaseExpr(scip, &terms[1]) );
    SCIP_CALL( SCIPreleaseExpr(scip, &terms[0]) );

    SCIP_CALL( SCIPcreateConsBasicNonlinear(scip, cons, name, expr, -SCIPinfinity(scip), rhsoffset) );

    SCIP_CALL( SCIPreleaseExpr(scip, &expr) );

    return SCIP_OKAY;
}