#ifndef HUNTERS_MILP_HEURISTICS_HPP
#define HUNTERS_MILP_HEURISTICS_HPP

#include "model/Model.hpp"
#include "numerical/Tolerances.hpp"
#include <vector>

namespace hunters {

struct HeuristicResult {
    bool found_feasible;
    double objective;
    std::vector<double> solution;

    HeuristicResult() : found_feasible(false), objective(1e30) {}
};

class PrimalRoundingHeuristic {
public:
    static HeuristicResult run(const Model& model,
                               const std::vector<double>& continuous_lp_solution,
                               const SolverTolerances& tol);
};

} // namespace hunters

#endif // HUNTERS_MILP_HEURISTICS_HPP
