
#include "pctsp/knapsack.hh"

using namespace std;

/** Solve a knapsack problem */
SCIP_RETCODE knapsack(std::vector<int>& costs, std::vector<int>& weights,
    int capacity) {
    // initialise the model
    SCIP* scip_model = NULL;
    SCIP_CALL(SCIPcreate(&scip_model));
    SCIP_CALL(SCIPincludeDefaultPlugins(scip_model));
    SCIP_CALL(SCIPcreateProbBasic(scip_model, "knapsack"));

    SCIP_CALL(SCIPsetObjsense(scip_model, SCIP_OBJSENSE_MAXIMIZE));
    // SCIP_CALL(SCIPsetObjsense(scip_model, SCIP_OBJSENSE_MINIMIZE));

    int num_variables = weights.size();

    // variable map
    std::map<int, SCIP_VAR*> edge_variable_map;

    // add variables
    int i = 0;
    for (const int& value : costs) {
        SCIP_VAR* variable;
        SCIP_CALL(SCIPcreateVar(scip_model, &variable, NULL, 0.0, 1.0, value,
            SCIP_VARTYPE_BINARY, TRUE, FALSE, NULL, NULL,
            NULL, NULL, NULL));
        SCIP_CALL(SCIPaddVar(scip_model, variable));
        edge_variable_map[i] = variable;
        i++;
    }
    SCIP_CONS* cons = nullptr;

    // convert the weight and capacity into 'long long int' type
    long long int long_weights[num_variables];
    for (int i = 0; i < num_variables; i++) {
        long_weights[i] = (long long int)weights[i];
    }
    long long int long_capacity = (long long int)capacity;

    // get the active variables in the problem and create a knapsack constraint
    SCIP_VAR** variables = SCIPgetVars(scip_model);
    SCIP_CALL(SCIPcreateConsBasicKnapsack(scip_model, &cons, "prize-constraint",
        num_variables, variables,
        long_weights, long_capacity));
    SCIP_CALL(SCIPaddCons(scip_model, cons));

    // print and release the knapsack constraint
    SCIP_CALL(SCIPreleaseCons(scip_model, &cons));

    // Solve the model
    SCIP_CALL(SCIPsolve(scip_model));

    // Get the solution
    SCIP_SOL* sol;

    for (int i = 0; i < num_variables; i++) {
        SCIP_VAR* variable = variables[i];
        SCIP_CALL(SCIPreleaseVar(scip_model, &variable));
    }
    SCIP_CALL(SCIPfree(&scip_model));
    BMScheckEmptyMemory();
    return SCIP_OKAY;
}
