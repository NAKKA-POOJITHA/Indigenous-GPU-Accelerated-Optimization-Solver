#ifndef HUNTERS_NUMERICAL_SOLUTION_VALIDATOR_HPP
#define HUNTERS_NUMERICAL_SOLUTION_VALIDATOR_HPP

#include "model/Model.hpp"
#include "numerical/Tolerances.hpp"
#include <vector>
#include <string>

namespace hunters {

struct ValidationReport {
    bool is_valid = false;
    double max_primal_residual = 0.0;
    int max_primal_residual_row = -1;
    double max_bound_violation = 0.0;
    int max_bound_violation_var = -1;
    double max_integrality_violation = 0.0;
    int max_integrality_violation_var = -1;
    double recalculated_objective = 0.0;
    double objective_discrepancy = 0.0;
    std::vector<std::string> violation_details;

    void print_report() const;
};

class SolutionValidator {
public:
    static ValidationReport validate(const Model& model,
                                    const std::vector<double>& solution,
                                    double reported_objective,
                                    const SolverTolerances& tol = SolverTolerances());
};

} // namespace hunters

#endif // HUNTERS_NUMERICAL_SOLUTION_VALIDATOR_HPP
