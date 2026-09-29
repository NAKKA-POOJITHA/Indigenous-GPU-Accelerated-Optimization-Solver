#include "sparse/SparseMatrix.hpp"
#include "sparse/VectorOps.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

namespace hunters {

void test_sparse_matrix() {
    std::cout << "[TEST] Running test_sparse_matrix..." << std::endl;

    // Create 3x3 matrix:
    // [ 10.0  0.0  2.0 ]
    // [  3.0  9.0  0.0 ]
    // [  0.0  7.0  8.0 ]
    std::vector<MatrixTriplet> triplets = {
        {0, 0, 10.0}, {0, 2, 2.0},
        {1, 0, 3.0},  {1, 1, 9.0},
        {2, 1, 7.0},  {2, 2, 8.0}
    };

    SparseMatrix A = SparseMatrix::from_triplets(3, 3, triplets);
    A.build_csc();

    assert(A.num_rows == 3);
    assert(A.num_cols == 3);
    assert(A.nnz() == 6);

    // Test element access
    assert(std::abs(A.get_element(0, 0) - 10.0) < 1e-12);
    assert(std::abs(A.get_element(0, 1) - 0.0) < 1e-12);
    assert(std::abs(A.get_element(1, 1) - 9.0) < 1e-12);
    assert(std::abs(A.get_element(2, 2) - 8.0) < 1e-12);

    // Test SpMV: y = A * x
    // x = [1, 2, 3]^T
    // Expected y = [10*1 + 2*3, 3*1 + 9*2, 7*2 + 8*3]^T = [16, 21, 38]^T
    std::vector<double> x = {1.0, 2.0, 3.0};
    std::vector<double> y;
    A.matvec(x, y);

    assert(std::abs(y[0] - 16.0) < 1e-12);
    assert(std::abs(y[1] - 21.0) < 1e-12);
    assert(std::abs(y[2] - 38.0) < 1e-12);

    // Test SpMV Transpose: y_t = A^T * x
    // A^T =
    // [ 10  3  0 ]
    // [  0  9  7 ]
    // [  2  0  8 ]
    // y_t = [10*1 + 3*2, 9*2 + 7*3, 2*1 + 8*3]^T = [16, 39, 26]^T
    std::vector<double> yt;
    A.matvec_transpose(x, yt);
    assert(std::abs(yt[0] - 16.0) < 1e-12);
    assert(std::abs(yt[1] - 39.0) < 1e-12);
    assert(std::abs(yt[2] - 26.0) < 1e-12);

    // Vector operations
    assert(std::abs(VectorOps::dot(x, x) - 14.0) < 1e-12);
    assert(std::abs(VectorOps::norm_inf(x) - 3.0) < 1e-12);

    std::cout << "  -> test_sparse_matrix PASSED" << std::endl;
}

} // namespace hunters
