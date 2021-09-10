#include "pctsp/exception.hh"
#include "pctsp/sciputils.hh"

std::string joinVariableNames(std::vector<SCIP_VAR*>& vars) {
    auto first = vars.begin();
    auto last = vars.end();
    return joinVariableNames(first, last);
}
