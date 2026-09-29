#ifndef HUNTERS_LP_SIMPLEX_SOLVER_HPP
#define HUNTERS_LP_SIMPLEX_SOLVER_HPP

#include "model/Model.hpp"
#include "sparse/SparseMatrix.hpp"
#include "lp/BasisMatrix.hpp"
#include "lp/Pricing.hpp"
#include "lp/RatioTest.hpp"
#include "numerical/Tolerances.hpp"
#include "numerical/Diagnostics.hpp"
#include <vector>
#include <string>

namespace hunters {

enum class SolverStatus {
    OPTIMAL,
    FEASIBLE,
    INFEASIBLE,
    UNBOUNDED,
    ITERATION_LIMIT,
    TIME_LIMIT,
    NUMERICAL_ERROR
};

struct SimplexResult {
    SolverStatus status;
    double objective_value;
    std::vector<double> primal_solution; // In terms of original model variables
    std::vector<double> dual_solution;   // Multipliers for constraints
    std::vector<double> reduced_costs;
    std::vector<int> basic_variables;
    SolverDiagnostics diagnostics;
    double solve_time_sec;

    SimplexResult()
        : status(SolverStatus::NUMERICAL_ERROR),
          objective_value(0.0),
          solve_time_sec(0.0) {}
};

struct StandardLP {
    SparseMatrix A;
    std::vector<double> b;
    std::vector<double> c;
    std::vector<double> lb;
    std::vector<double> ub;
    std::vector<double> var_shift; // x_orig = x_std + shift
    std::vector<int> orig_var_map; // maps std col to orig var idx (-1 if slack/artificial)
    std::vector<int> slack_rows;
    std::vector<int> artificial_rows;
    double obj_offset;
    ObjectiveSense orig_sense;
};

class SimplexSolver {
public:
    SolverTolerances tolerances;
    Pricing pricing;

    SimplexSolver(const SolverTolerances& tol = SolverTolerances(),
                  PricingStrategy strat = PricingStrategy::DEVEX_STEEPEST_EDGE)
        : tolerances(tol), pricing(strat) {}

    // Convert high-level Model into Standard LP form
    StandardLP transform_to_standard_form(const Model& model) const;

    // Solve LP using 2-Phase Revised Simplex
    SimplexResult solve(const Model& model);

    // Solve StandardLP with optional initial basis (warm-start)
    SimplexResult solve_standard_lp(const StandardLP& std_lp,
                                    const std::vector<int>& initial_basis = {});
};

} // namespace hunters

#endif // HUNTERS_LP_SIMPLEX_SOLVER_HPP
