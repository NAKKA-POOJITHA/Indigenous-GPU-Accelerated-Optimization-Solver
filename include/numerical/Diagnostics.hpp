#ifndef HUNTERS_NUMERICAL_DIAGNOSTICS_HPP
#define HUNTERS_NUMERICAL_DIAGNOSTICS_HPP

#include <string>
#include <vector>
#include <iostream>

namespace hunters {

struct SolverDiagnostics {
    int num_iterations = 0;
    int num_pivots = 0;
    int num_degenerate_pivots = 0;
    int num_refactorizations = 0;
    double max_condition_number = 1.0;
    double min_pivot_size = 1.0;
    double primal_residual = 0.0;
    double dual_residual = 0.0;
    double max_bound_violation = 0.0;
    double max_integrality_violation = 0.0;
    bool stalled = false;
    std::vector<std::string> warnings;

    void add_warning(const std::string& msg) {
        warnings.push_back(msg);
    }

    void print_diagnostics() const {
        std::cout << "----------- NUMERICAL DIAGNOSTICS -----------" << std::endl;
        std::cout << "Iterations          : " << num_iterations << std::endl;
        std::cout << "Degenerate Pivots   : " << num_degenerate_pivots << " ("
                  << (num_iterations > 0 ? (100.0 * num_degenerate_pivots / num_iterations) : 0.0) << "%)" << std::endl;
        std::cout << "Max Condition Est.  : " << max_condition_number << std::endl;
        std::cout << "Min Pivot Absolute  : " << min_pivot_size << std::endl;
        std::cout << "Primal Residual Inf : " << primal_residual << std::endl;
        std::cout << "Dual Residual Inf   : " << dual_residual << std::endl;
        std::cout << "Max Bound Violation : " << max_bound_violation << std::endl;
        if (max_integrality_violation > 0.0) {
            std::cout << "Integrality Viol.   : " << max_integrality_violation << std::endl;
        }
        if (!warnings.empty()) {
            std::cout << "Warnings Encountered: " << warnings.size() << std::endl;
            for (const auto& w : warnings) {
                std::cout << "  [WARNING] " << w << std::endl;
            }
        }
        std::cout << "---------------------------------------------" << std::endl;
    }
};

} // namespace hunters

#endif // HUNTERS_NUMERICAL_DIAGNOSTICS_HPP
