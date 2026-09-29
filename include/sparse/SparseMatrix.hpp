#ifndef HUNTERS_SPARSE_SPARSE_MATRIX_HPP
#define HUNTERS_SPARSE_SPARSE_MATRIX_HPP

#include <vector>
#include <iostream>
#include <stdexcept>
#include <cmath>

namespace hunters {

struct MatrixTriplet {
    int row;
    int col;
    double val;
    MatrixTriplet(int r = 0, int c = 0, double v = 0.0) : row(r), col(c), val(v) {}
};

class SparseMatrix {
public:
    int num_rows;
    int num_cols;

    // CSR format:
    std::vector<int> row_ptr;     // size num_rows + 1
    std::vector<int> col_indices; // size nnz
    std::vector<double> values;   // size nnz

    // CSC format (cached / built on demand for fast column operations):
    bool has_csc;
    std::vector<int> col_ptr;     // size num_cols + 1
    std::vector<int> row_indices; // size nnz
    std::vector<double> csc_values; // size nnz

    SparseMatrix(int rows = 0, int cols = 0)
        : num_rows(rows), num_cols(cols), has_csc(false) {
        row_ptr.assign(num_rows + 1, 0);
    }

    static SparseMatrix from_triplets(int rows, int cols, const std::vector<MatrixTriplet>& triplets);

    size_t nnz() const { return values.size(); }

    void build_csc();

    // SpMV: y = alpha * A * x + beta * y
    void matvec(const std::vector<double>& x, std::vector<double>& y, double alpha = 1.0, double beta = 0.0) const;

    // SpMV Transpose: y = alpha * A^T * x + beta * y
    void matvec_transpose(const std::vector<double>& x, std::vector<double>& y, double alpha = 1.0, double beta = 0.0) const;

    // Extract a specific column as dense or sparse
    void get_column_dense(int col, std::vector<double>& col_vec) const;
    void get_column_sparse(int col, std::vector<int>& rows, std::vector<double>& vals) const;

    // Extract a specific row as dense or sparse
    void get_row_dense(int row, std::vector<double>& row_vec) const;

    // Get element value (0 if not present)
    double get_element(int r, int c) const;

    // Transpose matrix
    SparseMatrix transpose() const;

    // Density
    double density() const {
        if (num_rows == 0 || num_cols == 0) return 0.0;
        return static_cast<double>(nnz()) / (static_cast<double>(num_rows) * static_cast<double>(num_cols));
    }
};

} // namespace hunters

#endif // HUNTERS_SPARSE_SPARSE_MATRIX_HPP
