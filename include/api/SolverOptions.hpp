#ifndef HUNTERS_API_SOLVER_OPTIONS_HPP
#define HUNTERS_API_SOLVER_OPTIONS_HPP

#include "numerical/Tolerances.hpp"
#include "lp/Pricing.hpp"
#include "milp/Node.hpp"
#include <string>

namespace hunters {

enum class SolverAlgorithm {
    AUTO,
    SIMPLEX,
    BRANCH_AND_BOUND
};

struct SolverOptions {
    SolverAlgorithm algorithm = SolverAlgorithm::AUTO;
    PricingStrategy pricing_strategy = PricingStrategy::DEVEX_STEEPEST_EDGE;
    NodeSelectionStrategy node_selection = NodeSelectionStrategy::BEST_BOUND;
    BranchingStrategy branching_strategy = BranchingStrategy::MOST_FRACTIONAL;

    bool enable_presolve = true;
    bool enable_scaling = true;
    bool enable_gpu = false;
    bool enable_heuristics = true;
    bool verbose = false;

    SolverTolerances tolerances;
};

} // namespace hunters

#endif // HUNTERS_API_SOLVER_OPTIONS_HPP
