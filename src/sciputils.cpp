#include "pctsp/exception.hh"
#include "pctsp/sciputils.hh"

std::string joinVariableNames(std::vector<SCIP_VAR*>& vars) {
    auto first = vars.begin();
    auto last = vars.end();
    return joinVariableNames(first, last);
}

void fillPositiveNegativeVars(
    VarVector& positive_vars,
    VarVector& negative_vars,
    VarVector& all_vars,
    std::vector<double>& var_coefs
) {
    assert (positive_vars.size() + negative_vars.size() == all_vars.size());
    assert (all_vars.size() == var_coefs.size());
    for (int i = 0; i < positive_vars.size(); i++)
        all_vars[i] = positive_vars[i];
    for (int i = positive_vars.size(); i < all_vars.size(); i++)
        all_vars[i] = negative_vars[i - positive_vars.size()];
    auto coef_it = var_coefs.begin();
    std::advance(coef_it, positive_vars.size());
    std::fill(var_coefs.begin(), coef_it, 1);
    std::fill(coef_it, var_coefs.end(), -1);
}

SCIP_RETCODE addRow(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    SCIP_RESULT* result,
    SCIP_SOL* sol,
    VarVector& var_vector,
    std::vector<double>& var_coefs,
    double& lhs,
    double& rhs,
    std::string& name
) {
    auto nvars = var_vector.size();
    double* vals = var_coefs.data();
    SCIP_VAR** vars = var_vector.data();

    SCIP_VAR* transvars[nvars];

    for (int i = 0; i < nvars; i++) {
        SCIP_VAR* transvar;
        SCIP_CALL(SCIPgetTransformedVar(scip, vars[i], &transvar));
        transvars[i] = transvar;
    }

    SCIP_ROW* row;
    SCIP_CALL(SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, name.c_str(), lhs, rhs, false, false, true));
    SCIP_CALL(SCIPcacheRowExtensions(scip, row));

    for (int i = 0; i < nvars; i++) {
        auto transvar = transvars[i];
        SCIPaddVarToRow(scip, row, transvar, vals[i]);
    }
    SCIP_CALL(SCIPflushRowExtensions(scip, row));

    if (SCIPisCutEfficacious(scip, sol, row)) {
        SCIP_Bool infeasible;
        SCIP_CALL(SCIPaddRow(scip, row, false, &infeasible));
        if (infeasible)
            *result = SCIP_CUTOFF;
        else
            *result = SCIP_SEPARATED;
    } 
    SCIP_CALL(SCIPreleaseRow(scip, &row));

    return SCIP_OKAY;
}
