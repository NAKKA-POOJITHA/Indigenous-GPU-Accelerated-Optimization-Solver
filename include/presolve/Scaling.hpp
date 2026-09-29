#ifndef HUNTERS_PRESOLVE_SCALING_HPP
#define HUNTERS_PRESOLVE_SCALING_HPP

#include "sparse/SparseMatrix.hpp"
#include <vector>
#include <iostream>

namespace hunters {

class Scaler {
public:
    int num_rows;
    int num_cols;
    std::vector<double> row_scales; // R_i
    std::vector<double> col_scales; // C_j
    bool is_scaled;

    Scaler(int rows = 0, int cols = 0)
        : num_rows(rows), num_cols(cols), is_scaled(false) {
        row_scales.assign(num_rows, 1.0);
        col_scales.assign(num_cols, 1.0);
    }

    // Compute geometric mean scaling followed by Ruiz equilibration
    void compute_scaling(const SparseMatrix& A, int ruiz_iterations = 3);

    // Apply scaling to matrix, rhs, objective, bounds
    SparseMatrix apply_scaling(const SparseMatrix& A,
                               std::vector<double>& rhs,
                               std::vector<double>& obj,
                               std::vector<double>& lb,
                               std::vector<double>& ub) const;

    // Unscale primal solution: x = C * x_scaled
    void unscale_primal(std::vector<double>& x) const;

    // Unscale dual solution: pi = R * pi_scaled
    void unscale_dual(std::vector<double>& pi) const;
};

} // namespace hunters

#endif // HUNTERS_PRESOLVE_SCALING_HPP
