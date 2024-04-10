#ifndef __PCTSP_ROBUST__
#define __PCTSP_ROBUST__

/** Distributionally Robust algorithms */

#include "algorithms.hh"

std::vector<std::pair<PCTSPvertex, PCTSPvertex>> DistRobustPrizeCollectingTsp(
    SCIP * scip,
    PCTSPgraph& graph,
    EdgeCostMap& cost_mean_map,
    EdgeCostMap& cost_var_map,
    VertexPrizeMap& prize_map,
    PrizeNumberType& quota,
    PCTSPvertex& root_vertex,
    std::string& name
);

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
);

#endif