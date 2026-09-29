#include "presolve/Scaling.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

namespace hunters {

void test_scaling() {
    std::cout << "[TEST] Running test_scaling..." << std::endl;

    // Badly scaled matrix with elements ranging from 1e-4 to 1e4
    std::vector<MatrixTriplet> triplets = {
        {0, 0, 1e4},  {0, 1, 1e-2},
        {1, 0, 1e-4}, {1, 1, 1e3}
    };

    SparseMatrix A = SparseMatrix::from_triplets(2, 2, triplets);
    std::vector<double> rhs = {100.0, 50.0};
    std::vector<double> obj = {1.0, 2.0};
    std::vector<double> lb = {0.0, 0.0};
    std::vector<double> ub = {100.0, 100.0};

    Scaler scaler(2, 2);
    scaler.compute_scaling(A, 3);

    assert(scaler.is_scaled);
    assert(scaler.row_scales.size() == 2);
    assert(scaler.col_scales.size() == 2);

    SparseMatrix A_scaled = scaler.apply_scaling(A, rhs, obj, lb, ub);

    // Verify dynamic range (max/min ratio) is significantly improved
    double min_v = 1e30, max_v = 0.0;
    for (double val : A_scaled.values) {
        double av = std::abs(val);
        min_v = std::min(min_v, av);
        max_v = std::max(max_v, av);
    }
    assert(min_v > 0.0);
    double ratio_before = 1e4 / 1e-4; // 1e8
    double ratio_after = max_v / min_v;
    assert(ratio_after < ratio_before); // Significantly better conditioned

    // Test unscaling
    std::vector<double> x_scaled = {2.0, 3.0};
    std::vector<double> x_orig = x_scaled;
    scaler.unscale_primal(x_orig);

    assert(std::abs(x_orig[0] - (x_scaled[0] * scaler.col_scales[0])) < 1e-12);
    assert(std::abs(x_orig[1] - (x_scaled[1] * scaler.col_scales[1])) < 1e-12);

    std::cout << "  -> test_scaling PASSED (Condition ratio reduced from " << ratio_before << " to " << ratio_after << ")" << std::endl;
}

} // namespace hunters
