#include "pctsp/heuristic.hh"
#include <scip/scipdefplugins.h>

void includeHeuristics(SCIP* scip) {
    // SCIPincludeHeurFeaspump(scip);
    // SCIPincludeHeurDps(scip);
}

float unitaryGain(int prize_v, CostNumberType cost_uw, CostNumberType cost_uv, CostNumberType cost_vw) {
    // v is the new vertex to insert (denoted j by Dell'Amico et al. [1998])
    // u and w already exist in the cycle (denoted h_i and h_{i+1} by Dell'Amico
    // et al. [1998])) cast to float to avoid rounding
    return (float)prize_v / (float)(cost_uv + cost_vw - cost_uw);
}

float unitaryLoss(
    int& external_path_prize,
    int& internal_path_prize,
    int& external_path_cost,
    int& internal_path_cost
) {
    return (float) (external_path_cost - internal_path_cost) / (float) (external_path_prize - internal_path_prize);
}

int numFeasibleExtensions(std::vector<bool>& is_feasible_extension) {
    int num_feasible_extensions = 0;
    for (bool is_feasible: is_feasible_extension) num_feasible_extensions += is_feasible;
    return num_feasible_extensions;
}

float averageUnitaryLoss(std::vector<float>& unitary_loss, std::vector<bool>& is_feasible_extension) {
    float total_loss = 0.0;
    for (int i = 0; i < unitary_loss.size(); i++) {
        if (is_feasible_extension[i]) total_loss += unitary_loss[i];
    }
    return total_loss / ((float) numFeasibleExtensions(is_feasible_extension));
}

int indexOfSmallestLoss(std::vector<float>& unitary_loss, std::vector<bool>& is_feasible_extension) {
    bool found_smallest_loss = false;
    float smallest_loss = 0.0;
    int index_of_smallest_loss = -1;
    for (int i = 0; i < unitary_loss.size(); i++) {
        if (is_feasible_extension[i] && ((unitary_loss[i] < smallest_loss) || !(found_smallest_loss))) {
            found_smallest_loss = true;
            smallest_loss = unitary_loss[i];
            index_of_smallest_loss = i;
        }
    }
    return index_of_smallest_loss;
}
