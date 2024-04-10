/** Distributionally Robust Algorithms */

#include <scip/cons_nonlinear.h>
#include <scip/expr_sum.h>

#include "pctsp/robust.hh"

void addDistRobustCons() {

}

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> DistRobustPrizeCollectingTsp(
    SCIP * scip,
    PCTSPgraph& graph,
    EdgeCostMap& cost_mean_map,
    EdgeCostMap& cost_var_map,
    VertexPrizeMap& prize_map,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name
) {
    // add variables, constraints and the SEC cutting plane
    std::vector<PCTSPedge> heuristic_edges = {};
    auto edge_var_map = modelPrizeCollectingTSP(
        scip, graph, heuristic_edges, cost_mean_map, prize_map, quota, root_vertex, name);
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
    int                   nvars,              /**< number of variables on left hand side of constraint (n) */
    SCIP_VAR**            vars,               /**< array with variables on left hand side (x_i) */
    SCIP_Real*            coefs,              /**< array with coefficients of left hand side variables (alpha_i), or NULL if all 1.0 */
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

    assert(vars != NULL || nvars == 0);

    SCIP_CALL( SCIPcreateExprSum(scip, &lhssum, 0, NULL, NULL, constant, NULL, NULL) );  /* gamma */
    for( i = 0; i < nvars; ++i )
    {
        SCIP_EXPR* varexpr;
        SCIP_EXPR* powexpr;

        SCIP_CALL( SCIPcreateExprVar(scip, &varexpr, vars[i], NULL, NULL) );   /* x_i */
        if( offsets != NULL && offsets[i] != 0.0 )
        {
            SCIP_EXPR* sum;
            SCIP_CALL( SCIPcreateExprSum(scip, &sum, 1, &varexpr, NULL, offsets[i], NULL, NULL) );  /* x_i + beta_i */
            SCIP_CALL( SCIPcreateExprPow(scip, &powexpr, sum, 2.0, NULL, NULL) );   /* (x_i + beta_i)^2 */
            SCIP_CALL( SCIPreleaseExpr(scip, &sum) );
        }
        else
        {
            SCIP_CALL( SCIPcreateExprPow(scip, &powexpr, varexpr, 2.0, NULL, NULL) );  /* x_i^2 */
        }

        SCIP_CALL( SCIPappendExprSumExpr(scip, lhssum, powexpr, coefs != NULL ? coefs[i]*coefs[i] : 1.0) );  /* + alpha_i^2 (x_i + beta_i)^2 */
        SCIP_CALL( SCIPreleaseExpr(scip, &varexpr) );
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