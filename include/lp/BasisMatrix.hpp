#ifndef HUNTERS_LP_BASIS_MATRIX_HPP
#define HUNTERS_LP_BASIS_MATRIX_HPP

#include "sparse/SparseMatrix.hpp"
#include "sparse/LinearAlgebra.hpp"
#include <vector>

namespace hunters {

class BasisMatrix {
public:
    int num_rows;
    int num_cols;
    std::vector<int> basis_indices;     // size m: column idx for each basic row
    std::vector<int> nonbasic_indices;  // column indices that are non-basic
    std::vector<int> var_status;        // 0 = basic, 1 = nonbasic at lower bound, 2 = nonbasic at upper bound

    DenseLU lu;
    int updates_since_refactorization;
    int max_updates_before_refactor;
    double current_rcond;

    BasisMatrix(int rows = 0, int cols = 0)
        : num_rows(rows), num_cols(cols),
          updates_since_refactorization(0),
          max_updates_before_refactor(50),
          current_rcond(1.0) {
        basis_indices.resize(num_rows, -1);
        var_status.resize(num_cols, 1);
    }

    // Refactorizes the basis matrix from the full coefficient matrix A
    bool refactorize(const SparseMatrix& A, double pivot_tol = 1e-12);

    // FTRAN: Solves B * d = A_col
    bool ftran(const std::vector<double>& a_col, std::vector<double>& d) const;

    // BTRAN: Solves B^T * pi = c_B
    bool btran(const std::vector<double>& c_B, std::vector<double>& pi) const;

    // Updates the basis: entering column enters at position p (leaving row p)
    bool update_basis(int leaving_row, int entering_col, const SparseMatrix& A);

    bool is_basic(int col_idx) const {
        return col_idx >= 0 && col_idx < static_cast<int>(var_status.size()) && var_status[col_idx] == 0;
    }
};

} // namespace hunters

#endif // HUNTERS_LP_BASIS_MATRIX_HPP
