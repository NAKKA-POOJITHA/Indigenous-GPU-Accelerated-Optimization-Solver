#include "lp/BasisMatrix.hpp"
#include <iostream>

namespace hunters {

bool BasisMatrix::refactorize(const SparseMatrix& A, double pivot_tol) {
    if (num_rows == 0) return true;

    // Construct dense B matrix: B[r][k] = A[r, basis_indices[k]]
    std::vector<std::vector<double>> B(num_rows, std::vector<double>(num_rows, 0.0));
    std::vector<double> col_vec;

    for (int k = 0; k < num_rows; ++k) {
        int col_idx = basis_indices[k];
        if (col_idx < 0 || col_idx >= A.num_cols) {
            return false;
        }
        A.get_column_dense(col_idx, col_vec);
        for (int r = 0; r < num_rows; ++r) {
            B[r][k] = col_vec[r];
        }
    }

    bool ok = lu.factorize(B, pivot_tol);
    current_rcond = lu.compute_rcond();
    updates_since_refactorization = 0;
    return ok;
}

bool BasisMatrix::ftran(const std::vector<double>& a_col, std::vector<double>& d) const {
    return lu.solve(a_col, d);
}

bool BasisMatrix::btran(const std::vector<double>& c_B, std::vector<double>& pi) const {
    return lu.solve_transpose(c_B, pi);
}

bool BasisMatrix::update_basis(int leaving_row, int entering_col, const SparseMatrix& A) {
    if (leaving_row < 0 || leaving_row >= num_rows) return false;

    int old_basic_var = basis_indices[leaving_row];
    basis_indices[leaving_row] = entering_col;

    var_status[old_basic_var] = 1; // Nonbasic
    var_status[entering_col] = 0;  // Basic

    updates_since_refactorization++;
    // Periodic refactorization for high numerical stability
    if (updates_since_refactorization >= max_updates_before_refactor) {
        return refactorize(A);
    } else {
        return refactorize(A); // Direct robust refactorization
    }
}

} // namespace hunters
