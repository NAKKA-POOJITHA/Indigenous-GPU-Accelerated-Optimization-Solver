#include "numerical/SolutionValidator.hpp"
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace hunters {

void ValidationReport::print_report() const {
    std::cout << "========== SOLUTION VALIDATION REPORT ==========" << std::endl;
    if (is_valid) {
        std::cout << "Status               : VALID SOLUTION [PASSED]" << std::endl;
    } else {
        std::cout << "Status               : SOLUTION VALIDATION FAILED [FAILED]" << std::endl;
    }
    std::cout << "Max Primal Residual  : " << std::scientific << std::setprecision(4) << max_primal_residual;
    if (max_primal_residual_row >= 0) std::cout << " (Row " << max_primal_residual_row << ")";
    std::cout << std::endl;

    std::cout << "Max Bound Violation  : " << std::scientific << std::setprecision(4) << max_bound_violation;
    if (max_bound_violation_var >= 0) std::cout << " (Var " << max_bound_violation_var << ")";
    std::cout << std::endl;

    std::cout << "Max Integrality Viol.: " << std::scientific << std::setprecision(4) << max_integrality_violation;
    if (max_integrality_violation_var >= 0) std::cout << " (Var " << max_integrality_violation_var << ")";
    std::cout << std::endl;

    std::cout << "Recalculated Obj     : " << std::fixed << std::setprecision(6) << recalculated_objective << std::endl;
    std::cout << "Objective Discrepancy: " << std::scientific << std::setprecision(4) << objective_discrepancy << std::endl;

    if (!violation_details.empty()) {
        std::cout << "Violations:" << std::endl;
        for (size_t i = 0; i < std::min(violation_details.size(), size_t(5)); ++i) {
            std::cout << "  - " << violation_details[i] << std::endl;
        }
        if (violation_details.size() > 5) {
            std::cout << "  - ... and " << (violation_details.size() - 5) << " more violations." << std::endl;
        }
    }
    std::cout << "================================================" << std::endl;
}

ValidationReport SolutionValidator::validate(const Model& model,
                                            const std::vector<double>& solution,
                                            double reported_objective,
                                            const SolverTolerances& tol) {
    ValidationReport report;
    report.is_valid = true;

    if (solution.size() != model.variables.size()) {
        report.is_valid = false;
        report.violation_details.push_back("Solution vector dimension mismatch (" +
            std::to_string(solution.size()) + " vs " + std::to_string(model.variables.size()) + ")");
        return report;
    }

    // 1. Check bounds & integrality
    for (size_t j = 0; j < model.variables.size(); ++j) {
        const auto& var = model.variables[j];
        double xj = solution[j];

        // Lower bound
        if (xj < var.lower_bound - tol.primal_feasibility_tolerance) {
            double viol = var.lower_bound - xj;
            if (viol > report.max_bound_violation) {
                report.max_bound_violation = viol;
                report.max_bound_violation_var = static_cast<int>(j);
            }
            report.is_valid = false;
            std::ostringstream ss;
            ss << "Var " << var.name << " (idx " << j << ") value " << xj << " < lower bound " << var.lower_bound;
            report.violation_details.push_back(ss.str());
        }

        // Upper bound
        if (xj > var.upper_bound + tol.primal_feasibility_tolerance) {
            double viol = xj - var.upper_bound;
            if (viol > report.max_bound_violation) {
                report.max_bound_violation = viol;
                report.max_bound_violation_var = static_cast<int>(j);
            }
            report.is_valid = false;
            std::ostringstream ss;
            ss << "Var " << var.name << " (idx " << j << ") value " << xj << " > upper bound " << var.upper_bound;
            report.violation_details.push_back(ss.str());
        }

        // Integrality
        if (var.is_integer()) {
            double rounded = std::round(xj);
            double int_viol = std::abs(xj - rounded);
            if (int_viol > report.max_integrality_violation) {
                report.max_integrality_violation = int_viol;
                report.max_integrality_violation_var = static_cast<int>(j);
            }
            if (int_viol > tol.integrality_tolerance) {
                report.is_valid = false;
                std::ostringstream ss;
                ss << "Integer var " << var.name << " (idx " << j << ") value " << xj << " is fractional (dist=" << int_viol << ")";
                report.violation_details.push_back(ss.str());
            }
        }
    }

    // 2. Check constraints
    for (size_t i = 0; i < model.constraints.size(); ++i) {
        const auto& constr = model.constraints[i];
        double lhs = 0.0;
        for (size_t k = 0; k < constr.var_indices.size(); ++k) {
            int v_idx = constr.var_indices[k];
            lhs += constr.coefficients[k] * solution[v_idx];
        }

        double viol = 0.0;
        if (constr.sense == ConstraintSense::LESS_EQUAL) {
            if (lhs > constr.rhs + tol.primal_feasibility_tolerance) {
                viol = lhs - constr.rhs;
            }
        } else if (constr.sense == ConstraintSense::GREATER_EQUAL) {
            if (lhs < constr.rhs - tol.primal_feasibility_tolerance) {
                viol = constr.rhs - lhs;
            }
        } else if (constr.sense == ConstraintSense::EQUAL) {
            viol = std::abs(lhs - constr.rhs);
            if (viol <= tol.primal_feasibility_tolerance) {
                viol = 0.0;
            }
        } else if (constr.sense == ConstraintSense::RANGE) {
            if (lhs < constr.lower_bound - tol.primal_feasibility_tolerance) {
                viol = constr.lower_bound - lhs;
            } else if (lhs > constr.upper_bound + tol.primal_feasibility_tolerance) {
                viol = lhs - constr.upper_bound;
            }
        }

        if (viol > report.max_primal_residual) {
            report.max_primal_residual = viol;
            report.max_primal_residual_row = static_cast<int>(i);
        }

        if (viol > tol.primal_feasibility_tolerance) {
            report.is_valid = false;
            std::ostringstream ss;
            ss << "Constraint " << constr.name << " (idx " << i << ") violated by " << viol << " (lhs=" << lhs << ", rhs=" << constr.rhs << ")";
            report.violation_details.push_back(ss.str());
        }
    }

    // 3. Recalculate objective
    double obj = model.obj_offset;
    for (size_t j = 0; j < model.variables.size(); ++j) {
        obj += model.variables[j].obj_coefficient * solution[j];
    }
    report.recalculated_objective = obj;
    report.objective_discrepancy = std::abs(obj - reported_objective);

    if (report.objective_discrepancy > 1e-4 * std::max(1.0, std::abs(reported_objective))) {
        report.is_valid = false;
        std::ostringstream ss;
        ss << "Objective mismatch: recalculated " << obj << " vs reported " << reported_objective;
        report.violation_details.push_back(ss.str());
    }

    return report;
}

} // namespace hunters
