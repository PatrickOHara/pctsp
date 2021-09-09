#ifndef __PCTSP_SCIPUTILS__
#define __PCTSP_SCIPUTILS__

#include <string>
#include <vector>
#include <objscip/objscip.h>

template<typename It>
std::string joinVariableNames(It& variable_first_it, It& variable_last_it) {
    std::string joined_names = "";
    std::string separator = "_";
    for (; variable_first_it != variable_last_it; variable_first_it++) {
        SCIP_VAR* var = *variable_first_it;
        if (var == NULL) {
            VariableIsNullException();
        }
        std::string var_name = SCIPvarGetName(var);
        joined_names += var_name + separator;
    }
    if (joined_names.size() > 0) joined_names.erase(joined_names.end()-1);
    return joined_names;
}

std::string joinVariableNames(std::vector<SCIP_VAR*>& vars);

#endif