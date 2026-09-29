#include "presolve/Scaling.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace hunters {

void Scaler::compute_scaling(const SparseMatrix& A, int ruiz_iterations) {
    num_rows = A.num_rows;
    num_cols = A.num_cols;
    row_scales.assign(num_rows, 1.0);
    col_scales.assign(num_cols, 1.0);
    is_scaled = true;

    if (num_rows == 0 || num_cols == 0 || A.values.empty()) {
        return;
    }

    // Step 1: Geometric Mean Scaling
    std::vector<double> row_min(num_rows, 1e30), row_max(num_rows, 0.0);
    for (int r = 0; r < num_rows; ++r) {
        int start = A.row_ptr[r];
        int end = A.row_ptr[r + 1];
        for (int k = start; k < end; ++k) {
            double v = std::abs(A.values[k]);
            if (v > 1e-15) {
                row_min[r] = std::min(row_min[r], v);
                row_max[r] = std::max(row_max[r], v);
            }
        }
        if (row_max[r] > 1e-15) {
            double gm = std::sqrt(row_min[r] * row_max[r]);
            if (gm > 1e-12) {
                row_scales[r] = 1.0 / gm;
            }
        }
    }

    // Column geometric mean scaling
    std::vector<double> col_min(num_cols, 1e30), col_max(num_cols, 0.0);
    for (int r = 0; r < num_rows; ++r) {
        int start = A.row_ptr[r];
        int end = A.row_ptr[r + 1];
        for (int k = start; k < end; ++k) {
            int c = A.col_indices[k];
            double v = std::abs(A.values[k]) * row_scales[r];
            if (v > 1e-15) {
                col_min[c] = std::min(col_min[c], v);
                col_max[c] = std::max(col_max[c], v);
            }
        }
    }
    for (int c = 0; c < num_cols; ++c) {
        if (col_max[c] > 1e-15) {
            double gm = std::sqrt(col_min[c] * col_max[c]);
            if (gm > 1e-12) {
                col_scales[c] = 1.0 / gm;
            }
        }
    }

    // Step 2: Ruiz Equilibration iterations (L_infinity norm scaling)
    for (int iter = 0; iter < ruiz_iterations; ++iter) {
        // Row L_inf
        for (int r = 0; r < num_rows; ++r) {
            double max_val = 0.0;
            int start = A.row_ptr[r];
            int end = A.row_ptr[r + 1];
            for (int k = start; k < end; ++k) {
                int c = A.col_indices[k];
                double v = std::abs(A.values[k]) * row_scales[r] * col_scales[c];
                max_val = std::max(max_val, v);
            }
            if (max_val > 1e-12) {
                double delta = 1.0 / std::sqrt(max_val);
                row_scales[r] *= delta;
            }
        }

        // Col L_inf
        std::vector<double> col_max_val(num_cols, 0.0);
        for (int r = 0; r < num_rows; ++r) {
            int start = A.row_ptr[r];
            int end = A.row_ptr[r + 1];
            for (int k = start; k < end; ++k) {
                int c = A.col_indices[k];
                double v = std::abs(A.values[k]) * row_scales[r] * col_scales[c];
                col_max_val[c] = std::max(col_max_val[c], v);
            }
        }
        for (int c = 0; c < num_cols; ++c) {
            if (col_max_val[c] > 1e-12) {
                double delta = 1.0 / std::sqrt(col_max_val[c]);
                col_scales[c] *= delta;
            }
        }
    }

    // Bound scales to prevent underflow/overflow
    for (double& r : row_scales) {
        r = std::max(1e-6, std::min(1e6, r));
    }
    for (double& c : col_scales) {
        c = std::max(1e-6, std::min(1e6, c));
    }
}

SparseMatrix Scaler::apply_scaling(const SparseMatrix& A,
                                   std::vector<double>& rhs,
                                   std::vector<double>& obj,
                                   std::vector<double>& lb,
                                   std::vector<double>& ub) const {
    if (!is_scaled) return A;

    // Scaled matrix: A'[r, c] = R[r] * A[r, c] * C[c]
    std::vector<MatrixTriplet> triplets;
    triplets.reserve(A.values.size());
    for (int r = 0; r < A.num_rows; ++r) {
        int start = A.row_ptr[r];
        int end = A.row_ptr[r + 1];
        double r_factor = (r < static_cast<int>(row_scales.size())) ? row_scales[r] : 1.0;
        for (int k = start; k < end; ++k) {
            int c = A.col_indices[k];
            double c_factor = (c < static_cast<int>(col_scales.size())) ? col_scales[c] : 1.0;
            triplets.emplace_back(r, c, A.values[k] * r_factor * c_factor);
        }
    }

    SparseMatrix scaled_A = SparseMatrix::from_triplets(A.num_rows, A.num_cols, triplets);
    scaled_A.build_csc();

    // Scale RHS: b' = R * b
    for (size_t r = 0; r < rhs.size(); ++r) {
        if (r < row_scales.size()) {
            rhs[r] *= row_scales[r];
        }
    }

    // Scale Objective: c' = C * c
    for (size_t c = 0; c < obj.size(); ++c) {
        if (c < col_scales.size()) {
            obj[c] *= col_scales[c];
        }
    }

    // Scale Bounds: l' = l / C, u' = u / C
    for (size_t c = 0; c < lb.size(); ++c) {
        if (c < col_scales.size()) {
            double c_factor = col_scales[c];
            if (lb[c] > -1e20) lb[c] /= c_factor;
            if (ub[c] < 1e20) ub[c] /= c_factor;
        }
    }

    return scaled_A;
}

void Scaler::unscale_primal(std::vector<double>& x) const {
    if (!is_scaled) return;
    for (size_t c = 0; c < x.size(); ++c) {
        if (c < col_scales.size()) {
            x[c] *= col_scales[c];
        }
    }
}

void Scaler::unscale_dual(std::vector<double>& pi) const {
    if (!is_scaled) return;
    for (size_t r = 0; r < pi.size(); ++r) {
        if (r < row_scales.size()) {
            pi[r] *= row_scales[r];
        }
    }
}

} // namespace hunters
