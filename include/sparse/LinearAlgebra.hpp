#ifndef HUNTERS_SPARSE_LINEAR_ALGEBRA_HPP
#define HUNTERS_SPARSE_LINEAR_ALGEBRA_HPP

#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace hunters {

class DenseLU {
public:
    int n;
    std::vector<std::vector<double>> LU;
    std::vector<int> pivot;
    bool is_singular;
    double rcond_estimate;

    DenseLU(int dim = 0) : n(dim), is_singular(false), rcond_estimate(1.0) {
        LU.assign(n, std::vector<double>(n, 0.0));
        pivot.assign(n, 0);
        for (int i = 0; i < n; ++i) pivot[i] = i;
    }

    // Factorizes matrix A into P * A = L * U
    bool factorize(const std::vector<std::vector<double>>& A, double pivot_tol = 1e-12);

    // Solve A * x = b  (i.e. L * U * x = P * b)
    bool solve(const std::vector<double>& b, std::vector<double>& x) const;

    // Solve A^T * y = c (i.e. U^T * L^T * P * y = c)
    bool solve_transpose(const std::vector<double>& c, std::vector<double>& y) const;

    // Compute estimate of condition number
    double compute_rcond() const;
};

// Eta matrix for Product Form of the Inverse (PFI) updates:
// Represents an elementary matrix E_k where column p is replaced by eta column
struct EtaMatrix {
    int pivot_row;
    std::vector<int> nonzeros_idx;
    std::vector<double> nonzeros_val;
    double diag_val; // 1 / pivot_element

    // Apply E * x
    void apply_forward(std::vector<double>& x) const;

    // Apply x^T * E (or E^T * x)
    void apply_transpose(std::vector<double>& x) const;
};

} // namespace hunters

#endif // HUNTERS_SPARSE_LINEAR_ALGEBRA_HPP
