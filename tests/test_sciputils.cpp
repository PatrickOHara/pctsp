#include <gtest/gtest.h>
#include <objscip/objscipdefplugins.h>
#include "pctsp/sciputils.hh"

TEST(TestSCIPutils, testFillPositiveNegativeVars) {
    // create linear program
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);
    SCIPcreateProbBasic(scip, "test-insert-edge-vertex-variables");

    int n_edge_vars = 4;
    int n_vertex_vars = 2;
    int nvars = n_edge_vars + n_vertex_vars;

    VarVector edge_variables(n_edge_vars);
    VarVector vertex_variables(n_vertex_vars);
    VarVector all_variables(nvars);
    std::vector<double> var_coefs(nvars);

    for (int i = 0; i < n_edge_vars; i++) {
        SCIP_VAR* var = NULL;
        SCIPcreateVarBasic(scip, &var, NULL, 0, 1, 1, SCIP_VARTYPE_CONTINUOUS);
        SCIPaddVar(scip, var);
        edge_variables[i] = var;
    }
    for (int i = 0; i < n_vertex_vars; i++) {
        SCIP_VAR* var = NULL;
        SCIPcreateVarBasic(scip, &var, NULL, 0, 1, 1, SCIP_VARTYPE_CONTINUOUS);
        SCIPaddVar(scip, var);
        vertex_variables[i] = var;
    }
    // run insertion algorithm
    fillPositiveNegativeVars(edge_variables, vertex_variables, all_variables, var_coefs);
    EXPECT_EQ(edge_variables.size(), n_edge_vars);
    EXPECT_EQ(vertex_variables.size(), n_vertex_vars);
    EXPECT_EQ(all_variables.size(), nvars);
    EXPECT_EQ(var_coefs.size(), nvars);
    for (int i = 0; i < n_edge_vars; i++) {
        EXPECT_EQ(var_coefs[i], 1);
        EXPECT_EQ(edge_variables[i], all_variables[i]);
    }
    for (int i = n_edge_vars; i < nvars; i++) {
        EXPECT_EQ(var_coefs[i], -1);
        EXPECT_EQ(vertex_variables[i - n_edge_vars], all_variables[i]);
    }
    for (SCIP_VAR* var : all_variables) {
        SCIPreleaseVar(scip, &var);
    }
    SCIPfree(&scip);
}