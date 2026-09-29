#ifndef HUNTERS_MILP_BRANCH_AND_BOUND_HPP
#define HUNTERS_MILP_BRANCH_AND_BOUND_HPP

#include "model/Model.hpp"
#include "lp/SimplexSolver.hpp"
#include "milp/Node.hpp"
#include "milp/BranchingRules.hpp"
#include "milp/Heuristics.hpp"
#include "numerical/Tolerances.hpp"
#include <vector>
#include <queue>
#include <memory>

namespace hunters {

struct MILPResult {
    SolverStatus status;
    double incumbent_objective;
    double best_bound;
    double mip_gap;
    int node_count;
    int total_lp_iterations;
    std::vector<double> best_solution;
    double solve_time_sec;
    SolverDiagnostics diagnostics;

    MILPResult()
        : status(SolverStatus::NUMERICAL_ERROR),
          incumbent_objective(1e30),
          best_bound(-1e30),
          mip_gap(1.0),
          node_count(0),
          total_lp_iterations(0),
          solve_time_sec(0.0) {}
};

class BranchAndBoundSolver {
public:
    SolverTolerances tolerances;
    NodeSelectionStrategy node_strategy;
    BranchingStrategy branching_strategy;
    bool enable_heuristics;

    BranchAndBoundSolver(const SolverTolerances& tol = SolverTolerances(),
                         NodeSelectionStrategy n_strat = NodeSelectionStrategy::BEST_BOUND,
                         BranchingStrategy b_strat = BranchingStrategy::MOST_FRACTIONAL)
        : tolerances(tol), node_strategy(n_strat), branching_strategy(b_strat), enable_heuristics(true) {}

    MILPResult solve(const Model& model);
};

} // namespace hunters

#endif // HUNTERS_MILP_BRANCH_AND_BOUND_HPP
