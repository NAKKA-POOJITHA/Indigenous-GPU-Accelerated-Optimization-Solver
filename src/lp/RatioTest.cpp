#include "lp/RatioTest.hpp"
#include <cmath>
#include <algorithm>

namespace hunters {

RatioTestResult RatioTest::harris_ratio_test(const std::vector<double>& x_B,
                                            const std::vector<double>& d,
                                            const std::vector<int>& basis_indices,
                                            const std::vector<double>& lower_bounds,
                                            const std::vector<double>& upper_bounds,
                                            int entering_col,
                                            double feasibility_tol,
                                            double pivot_tol) {
    RatioTestResult res;
    res.leaving_row = -1;
    res.step_length = 1e30;
    res.is_unbounded = true;
    res.is_bound_flip = false;
    res.pivot_element = 0.0;

    int m = static_cast<int>(x_B.size());

    // Check if entering variable has a finite upper bound
    double entering_ub = (entering_col < static_cast<int>(upper_bounds.size())) ? upper_bounds[entering_col] : 1e30;
    double entering_lb = (entering_col < static_cast<int>(lower_bounds.size())) ? lower_bounds[entering_col] : 0.0;
    double max_entering_step = entering_ub - entering_lb;

    // Pass 1: Find maximum permissible step theta_max with tolerance
    double theta_max = max_entering_step;

    for (int i = 0; i < m; ++i) {
        double d_i = d[i];
        int var_i = basis_indices[i];
        double ub_i = (var_i < static_cast<int>(upper_bounds.size())) ? upper_bounds[var_i] : 1e30;
        double lb_i = (var_i < static_cast<int>(lower_bounds.size())) ? lower_bounds[var_i] : 0.0;

        if (d_i > pivot_tol) {
            // Variable decreases towards lower bound: x_B[i] - theta * d_i >= lb_i - tol
            double theta_i = (x_B[i] - lb_i + feasibility_tol) / d_i;
            if (theta_i < theta_max) {
                theta_max = theta_i;
                res.is_unbounded = false;
            }
        } else if (d_i < -pivot_tol && ub_i < 1e20) {
            // Variable increases towards upper bound: x_B[i] - theta * d_i <= ub_i + tol
            double theta_i = (x_B[i] - ub_i - feasibility_tol) / d_i; // Note d_i is negative
            if (theta_i < theta_max) {
                theta_max = theta_i;
                res.is_unbounded = false;
            }
        }
    }

    if (theta_max >= 1e25 && max_entering_step >= 1e25) {
        res.is_unbounded = true;
        return res;
    }

    // Check if bound flip on entering variable is limiting
    if (max_entering_step <= theta_max) {
        res.is_bound_flip = true;
        res.step_length = max_entering_step;
        res.is_unbounded = false;
        return res;
    }

    // Pass 2: Pick candidate with LARGEST pivot element |d_i| among valid candidates
    int best_row = -1;
    double largest_pivot = 0.0;
    double exact_step = 0.0;

    for (int i = 0; i < m; ++i) {
        double d_i = d[i];
        int var_i = basis_indices[i];
        double ub_i = (var_i < static_cast<int>(upper_bounds.size())) ? upper_bounds[var_i] : 1e30;
        double lb_i = (var_i < static_cast<int>(lower_bounds.size())) ? lower_bounds[var_i] : 0.0;

        if (d_i > pivot_tol) {
            double raw_ratio = (x_B[i] - lb_i) / d_i;
            if (raw_ratio <= theta_max) {
                if (std::abs(d_i) > largest_pivot) {
                    largest_pivot = std::abs(d_i);
                    best_row = i;
                    exact_step = std::max(0.0, raw_ratio);
                }
            }
        } else if (d_i < -pivot_tol && ub_i < 1e20) {
            double raw_ratio = (x_B[i] - ub_i) / d_i;
            if (raw_ratio <= theta_max) {
                if (std::abs(d_i) > largest_pivot) {
                    largest_pivot = std::abs(d_i);
                    best_row = i;
                    exact_step = std::max(0.0, raw_ratio);
                }
            }
        }
    }

    res.leaving_row = best_row;
    res.step_length = exact_step;
    res.pivot_element = (best_row >= 0) ? d[best_row] : 0.0;
    res.is_unbounded = (best_row == -1);
    return res;
}

} // namespace hunters
