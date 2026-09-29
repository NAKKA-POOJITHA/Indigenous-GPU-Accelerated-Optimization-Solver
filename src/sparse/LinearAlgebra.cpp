#include "sparse/LinearAlgebra.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace hunters {

bool DenseLU::factorize(const std::vector<std::vector<double>>& A, double pivot_tol) {
    n = static_cast<int>(A.size());
    if (n == 0) return true;
    LU = A;
    pivot.resize(n);
    for (int i = 0; i < n; ++i) pivot[i] = i;
    is_singular = false;

    // Track row norms for Markowitz-like / scaled partial pivoting
    std::vector<double> row_scale(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double max_val = 0.0;
        for (int j = 0; j < n; ++j) {
            max_val = std::max(max_val, std::abs(LU[i][j]));
        }
        if (max_val < 1e-15) {
            is_singular = true;
            rcond_estimate = 0.0;
            return false;
        }
        row_scale[i] = 1.0 / max_val;
    }

    double min_diag = 1e30;
    double max_diag = 0.0;

    for (int k = 0; k < n; ++k) {
        // Find pivot with column partial pivoting
        int best_row = k;
        double best_val = std::abs(LU[k][k]) * row_scale[k];

        for (int i = k + 1; i < n; ++i) {
            double candidate = std::abs(LU[i][k]) * row_scale[i];
            if (candidate > best_val) {
                best_val = candidate;
                best_row = i;
            }
        }

        if (std::abs(LU[best_row][k]) < pivot_tol) {
            is_singular = true;
            rcond_estimate = 0.0;
            return false;
        }

        // Swap rows if needed
        if (best_row != k) {
            std::swap(LU[k], LU[best_row]);
            std::swap(pivot[k], pivot[best_row]);
            std::swap(row_scale[k], row_scale[best_row]);
        }

        double diag = LU[k][k];
        double abs_diag = std::abs(diag);
        min_diag = std::min(min_diag, abs_diag);
        max_diag = std::max(max_diag, abs_diag);

        // Elimination
        for (int i = k + 1; i < n; ++i) {
            LU[i][k] /= diag;
            double factor = LU[i][k];
            for (int j = k + 1; j < n; ++j) {
                LU[i][j] -= factor * LU[k][j];
            }
        }
    }

    rcond_estimate = (max_diag > 1e-15) ? (min_diag / max_diag) : 0.0;
    return true;
}

bool DenseLU::solve(const std::vector<double>& b, std::vector<double>& x) const {
    if (is_singular || n == 0) return false;
    x.resize(n);

    // Apply permutation P*b
    std::vector<double> pb(n);
    for (int i = 0; i < n; ++i) {
        pb[i] = b[pivot[i]];
    }

    // Forward substitution: L * y = P * b
    std::vector<double> y(n);
    for (int i = 0; i < n; ++i) {
        double sum = pb[i];
        for (int j = 0; j < i; ++j) {
            sum -= LU[i][j] * y[j];
        }
        y[i] = sum;
    }

    // Backward substitution: U * x = y
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= LU[i][j] * x[j];
        }
        if (std::abs(LU[i][i]) < 1e-15) {
            return false;
        }
        x[i] = sum / LU[i][i];
    }

    return true;
}

bool DenseLU::solve_transpose(const std::vector<double>& c, std::vector<double>& y) const {
    if (is_singular || n == 0) return false;
    y.resize(n);

    // System: A^T y = c  =>  (P^T L U)^T y = c  =>  U^T L^T P y = c
    // 1. Solve U^T * z = c (Forward solve with transpose of U)
    std::vector<double> z(n);
    for (int i = 0; i < n; ++i) {
        double sum = c[i];
        for (int j = 0; j < i; ++j) {
            sum -= LU[j][i] * z[j]; // U^T[i][j] = U[j][i]
        }
        if (std::abs(LU[i][i]) < 1e-15) return false;
        z[i] = sum / LU[i][i];
    }

    // 2. Solve L^T * w = z (Backward solve with transpose of unit lower L)
    std::vector<double> w(n);
    for (int i = n - 1; i >= 0; --i) {
        double sum = z[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= LU[j][i] * w[j]; // L^T[i][j] = L[j][i]
        }
        w[i] = sum;
    }

    // 3. Apply inverse permutation: P y = w  =>  y[pivot[i]] = w[i]
    for (int i = 0; i < n; ++i) {
        y[pivot[i]] = w[i];
    }

    return true;
}

double DenseLU::compute_rcond() const {
    return rcond_estimate;
}

void EtaMatrix::apply_forward(std::vector<double>& x) const {
    // E * x: column pivot_row is eta column
    double xp = x[pivot_row];
    x[pivot_row] = xp * diag_val;
    for (size_t i = 0; i < nonzeros_idx.size(); ++i) {
        int r = nonzeros_idx[i];
        if (r != pivot_row) {
            x[r] += nonzeros_val[i] * x[pivot_row];
        }
    }
}

void EtaMatrix::apply_transpose(std::vector<double>& x) const {
    // E^T * x
    double sum = x[pivot_row] * diag_val;
    for (size_t i = 0; i < nonzeros_idx.size(); ++i) {
        int r = nonzeros_idx[i];
        if (r != pivot_row) {
            sum += nonzeros_val[i] * diag_val * x[r];
        }
    }
    x[pivot_row] = sum;
}

} // namespace hunters
