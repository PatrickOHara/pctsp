#include <gtest/gtest.h>
#include <objscip/objscipdefplugins.h>
#include <scip/scipsdpdefplugins.h>
#include <scip/cons_sdp.h>
#include "pctsp/sciputils.hh"
#include "pctsp/robust.hh"

TEST(TestSoc, testSoc) {
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);
    SCIPcreateProbBasic(scip, "test-scip-soc");
    SCIPsetObjsense(scip, SCIP_Objsense::SCIP_OBJSENSE_MINIMIZE);

    int nVars = 4;
    VarVector vars (nVars);
    for (auto& var : vars) {
        SCIPcreateVarBasic(scip, &var, NULL, 0, 1, 0, SCIP_Vartype::SCIP_VARTYPE_BINARY);
        SCIPaddVar(scip, var);
    }

    SCIP_VAR* t = NULL;
    SCIP_VAR* z = NULL;
    SCIPcreateVarBasic(scip, &t, "t", 0, SCIPinfinity(scip), 1, SCIP_Vartype::SCIP_VARTYPE_CONTINUOUS);
    SCIPcreateVarBasic(scip, &z, "z", 0, SCIPinfinity(scip), 1, SCIP_Vartype::SCIP_VARTYPE_CONTINUOUS);
    SCIPaddVar(scip, t);
    SCIPaddVar(scip, z);

    std::string consName = "soc";
    SCIP_CONS* socCons = NULL;

    // SCIPcreateConsBasicSOCNonlinear(scip, &socCons, consName.c_str(), nVars, vars.data(), NULL, NULL, 0, t, 1, 0);
    std::vector<SCIP_VAR*> rhsVars = {t, z};
    std::vector<double> rhsCoefs = {1, 1};
    SCIPcreateConsSOC(scip, &socCons, consName.c_str(), nVars, vars.data(), NULL, NULL, 0, rhsVars.size(), rhsVars.data(), rhsCoefs.data(), 0);
    SCIPaddCons(scip, socCons);

    SCIP_CONS* knapsack = NULL;
    std::vector<double> linWeights (nVars);
    for (int i = 0; i < nVars; i++) {
        linWeights[i] = -1;
    }
    // SCIPcreateConsBasicKnapsack(scip, &knapsack, "knapsack", nVars, vars.data(), kpWeights.data(), -nVars + 1);
    SCIPcreateConsBasicLinear(scip, &knapsack, "linear", nVars, vars.data(), linWeights.data(), - SCIPinfinity(scip), -nVars+1);
    SCIPaddCons(scip, knapsack);

    SCIPsolve(scip);
}

TEST(TestScipSdp, testSdp) {
    SCIP* scip = NULL;
    SCIPcreate(&scip);
    SCIPSDPincludeDefaultPlugins(scip);
    SCIPcreateProbBasic(scip, "test-scip-sdp");
    SCIPsetObjsense(scip, SCIP_Objsense::SCIP_OBJSENSE_MINIMIZE);


    int nVars = 4;
    VarVector varVector (nVars);
    for (auto& var : varVector) {
        SCIPcreateVarBasic(scip, &var, NULL, 0, 1, 0, SCIP_Vartype::SCIP_VARTYPE_BINARY);
        SCIPaddVar(scip, var);
    }
    auto vars = varVector.data();
    SCIP_VAR* t = NULL;
    SCIPcreateVarBasic(scip, &t, "t", 0, SCIPinfinity(scip), 1, SCIP_Vartype::SCIP_VARTYPE_CONTINUOUS);

    // create a semi-definite constraint
    std::string name = "SDP-constraint";
    SCIP_CONS* sdpCons = NULL;
    std::vector<int> numVarNonZero = {4, 4, 4, 4};
    std::vector<std::vector<int>> nonZeroColsIndices = {{0,0,0,0}, {1}, {2}, {3}};
    std::vector<std::vector<int>> nonZeroRowIndices = {{0, 1, 2, 3}};
    std::vector<double> coefs = {1.0, 2.5, 3.5, 4.5};

    int constnnonz = 0;         /**< number of nonzeros in the constant part of this SDP constraint */
    int* constcol = {};           /**< column indices of the constant nonzeros */
    int* constrow = {};           /**< row indices of the constant nonzeros */
    SCIP_Real* constval = {};           /**< values of the constant nonzeros */

/*
    SCIPcreateConsSdp(scip,
        &sdpCons,
        name.c_str(),
        nVars,
        nVars,
        nVars,
        numVarNonZero.data(),
        nonZeroColsIndices.data(),
        nonZeroRowIndices.data(),
        coefs.data(),
        vars,
        constnnonz,
        constcol,
        constrow,
        constval,
        0
    );
    SCIPaddCons(scip, sdpCons);
    EXPECT_EQ(SCIPconsSdpGetNVars(scip, sdpCons), nVars);*/
}
