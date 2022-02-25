#include <scip/scipdefplugins.h>
#include "pctsp/node_selection.hh"

void includeNodeSelection(SCIP* scip) {
    SCIPincludeNodeselBreadthfirst(scip);
    SCIPincludeNodeselDfs(scip);
    SCIPincludeNodeselEstimate(scip);
}