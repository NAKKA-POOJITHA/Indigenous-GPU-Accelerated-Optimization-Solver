#include "lp/Pricing.hpp"
#include <cmath>
#include <algorithm>

namespace hunters {

void Pricing::initialize_weights(int num_cols) {
    devex_weights.assign(num_cols, 1.0);
}

int Pricing::select_entering_variable(const SparseMatrix& A,
                                     const std::vector<double>& c,
                                     const std::vector<double>& pi,
                                     const BasisMatrix& basis,
                                     const std::vector<double>& lower_bounds,
                                     const std::vector<double>& upper_bounds,
                                     const std::vector<double>& x_vals,
                                     double optimality_tol,
                                     std::vector<double>& reduced_costs) {
    int num_cols = A.num_cols;
    reduced_costs.resize(num_cols);

    if (devex_weights.size() != static_cast<size_t>(num_cols)) {
        initialize_weights(num_cols);
    }

    int best_entering = -1;
    double best_score = 0.0;

    for (int j = 0; j < num_cols; ++j) {
        if (basis.is_basic(j)) {
            reduced_costs[j] = 0.0;
            continue;
        }

        // Reduced cost d_j = c_j - pi^T * A_{.j}
        double pi_dot_a = 0.0;
        if (A.has_csc) {
            int start = A.col_ptr[j];
            int end = A.col_ptr[j + 1];
            for (int k = start; k < end; ++k) {
                int r = A.row_indices[k];
                if (r < static_cast<int>(pi.size())) {
                    pi_dot_a += pi[r] * A.csc_values[k];
                }
            }
        } else {
            for (int r = 0; r < A.num_rows; ++r) {
                double a_rj = A.get_element(r, j);
                if (std::abs(a_rj) > 1e-15 && r < static_cast<int>(pi.size())) {
                    pi_dot_a += pi[r] * a_rj;
                }
            }
        }

        double c_j = (j < static_cast<int>(c.size())) ? c[j] : 0.0;
        double dj = c_j - pi_dot_a;
        reduced_costs[j] = dj;

        double xj = (j < static_cast<int>(x_vals.size())) ? x_vals[j] : 0.0;
        double lb = (j < static_cast<int>(lower_bounds.size())) ? lower_bounds[j] : 0.0;
        double ub = (j < static_cast<int>(upper_bounds.size())) ? upper_bounds[j] : 1e30;

        bool can_increase = (xj < ub - optimality_tol);
        bool can_decrease = (xj > lb + optimality_tol);

        if (strategy == PricingStrategy::BLANDS_RULE) {
            if (can_increase && dj < -optimality_tol) {
                return j; // First improving candidate
            }
            if (can_decrease && dj > optimality_tol) {
                return j;
            }
            continue;
        }

        if (can_increase && dj < -optimality_tol) {
            double violation = -dj;
            double score = violation;
            if (strategy == PricingStrategy::DEVEX_STEEPEST_EDGE) {
                double w = std::max(1e-4, devex_weights[j]);
                score = (violation * violation) / w;
            }
            if (score > best_score) {
                best_score = score;
                best_entering = j;
            }
        } else if (can_decrease && dj > optimality_tol) {
            double violation = dj;
            double score = violation;
            if (strategy == PricingStrategy::DEVEX_STEEPEST_EDGE) {
                double w = std::max(1e-4, devex_weights[j]);
                score = (violation * violation) / w;
            }
            if (score > best_score) {
                best_score = score;
                best_entering = j;
            }
        }
    }

    return best_entering;
}

void Pricing::update_weights(int entering_col, int leaving_row, const std::vector<double>& d) {
    if (strategy != PricingStrategy::DEVEX_STEEPEST_EDGE) return;
    if (entering_col >= 0 && entering_col < static_cast<int>(devex_weights.size())) {
        double d_norm_sq = 0.0;
        for (double val : d) {
            d_norm_sq += val * val;
        }
        devex_weights[entering_col] = std::max(1.0, d_norm_sq + 1.0);
    }
}

} // namespace hunters
