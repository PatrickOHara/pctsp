#include "pctsp/heuristic.hh"

float unitary_gain(int prize_v, int cost_uw, int cost_uv, int cost_vw) {
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
