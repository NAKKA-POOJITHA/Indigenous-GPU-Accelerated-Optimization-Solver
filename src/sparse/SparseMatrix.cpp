#include "sparse/SparseMatrix.hpp"
#include <algorithm>
#include <map>

namespace hunters {

SparseMatrix SparseMatrix::from_triplets(int rows, int cols, const std::vector<MatrixTriplet>& triplets) {
    SparseMatrix mat(rows, cols);
    if (triplets.empty()) {
        return mat;
    }

    // Sort triplets by row, then col
    std::vector<MatrixTriplet> sorted = triplets;
    std::sort(sorted.begin(), sorted.end(), [](const MatrixTriplet& a, const MatrixTriplet& b) {
        if (a.row != b.row) return a.row < b.row;
        return a.col < b.col;
    });

    // Merge duplicates
    std::vector<int> row_counts(rows, 0);
    std::vector<MatrixTriplet> unique_triplets;
    for (size_t i = 0; i < sorted.size(); ++i) {
        if (sorted[i].row < 0 || sorted[i].row >= rows || sorted[i].col < 0 || sorted[i].col >= cols) {
            continue; // Skip out of bound
        }
        if (std::abs(sorted[i].val) < 1e-15) {
            continue;
        }
        if (!unique_triplets.empty() &&
            unique_triplets.back().row == sorted[i].row &&
            unique_triplets.back().col == sorted[i].col) {
            unique_triplets.back().val += sorted[i].val;
        } else {
            unique_triplets.push_back(sorted[i]);
        }
    }

    for (const auto& t : unique_triplets) {
        if (std::abs(t.val) > 1e-15) {
            row_counts[t.row]++;
        }
    }

    mat.row_ptr.assign(rows + 1, 0);
    for (int r = 0; r < rows; ++r) {
        mat.row_ptr[r + 1] = mat.row_ptr[r] + row_counts[r];
    }

    mat.col_indices.reserve(unique_triplets.size());
    mat.values.reserve(unique_triplets.size());

    for (const auto& t : unique_triplets) {
        if (std::abs(t.val) > 1e-15) {
            mat.col_indices.push_back(t.col);
            mat.values.push_back(t.val);
        }
    }

    return mat;
}

void SparseMatrix::build_csc() {
    if (has_csc && col_ptr.size() == static_cast<size_t>(num_cols + 1)) return;

    col_ptr.assign(num_cols + 1, 0);
    std::vector<int> col_counts(num_cols, 0);

    for (int c : col_indices) {
        if (c >= 0 && c < num_cols) {
            col_counts[c]++;
        }
    }

    for (int c = 0; c < num_cols; ++c) {
        col_ptr[c + 1] = col_ptr[c] + col_counts[c];
    }

    std::vector<int> current_pos = col_ptr;
    row_indices.resize(values.size());
    csc_values.resize(values.size());

    for (int r = 0; r < num_rows; ++r) {
        for (int k = row_ptr[r]; k < row_ptr[r + 1]; ++k) {
            int c = col_indices[k];
            double v = values[k];
            int dest = current_pos[c]++;
            row_indices[dest] = r;
            csc_values[dest] = v;
        }
    }

    has_csc = true;
}

void SparseMatrix::matvec(const std::vector<double>& x, std::vector<double>& y, double alpha, double beta) const {
    if (y.size() != static_cast<size_t>(num_rows)) {
        y.resize(num_rows, 0.0);
    }
    for (int r = 0; r < num_rows; ++r) {
        double sum = 0.0;
        int row_start = row_ptr[r];
        int row_end = row_ptr[r + 1];
        for (int k = row_start; k < row_end; ++k) {
            sum += values[k] * x[col_indices[k]];
        }
        if (beta == 0.0) {
            y[r] = alpha * sum;
        } else {
            y[r] = alpha * sum + beta * y[r];
        }
    }
}

void SparseMatrix::matvec_transpose(const std::vector<double>& x, std::vector<double>& y, double alpha, double beta) const {
    if (y.size() != static_cast<size_t>(num_cols)) {
        y.resize(num_cols, 0.0);
    }
    if (beta == 0.0) {
        std::fill(y.begin(), y.end(), 0.0);
    } else if (beta != 1.0) {
        for (double& v : y) v *= beta;
    }

    for (int r = 0; r < num_rows; ++r) {
        double xr = x[r];
        if (std::abs(xr) < 1e-15) continue;
        int row_start = row_ptr[r];
        int row_end = row_ptr[r + 1];
        for (int k = row_start; k < row_end; ++k) {
            int c = col_indices[k];
            y[c] += alpha * xr * values[k];
        }
    }
}

void SparseMatrix::get_column_dense(int col, std::vector<double>& col_vec) const {
    col_vec.assign(num_rows, 0.0);
    if (has_csc) {
        int start = col_ptr[col];
        int end = col_ptr[col + 1];
        for (int k = start; k < end; ++k) {
            col_vec[row_indices[k]] = csc_values[k];
        }
    } else {
        for (int r = 0; r < num_rows; ++r) {
            for (int k = row_ptr[r]; k < row_ptr[r + 1]; ++k) {
                if (col_indices[k] == col) {
                    col_vec[r] = values[k];
                    break;
                }
            }
        }
    }
}

void SparseMatrix::get_column_sparse(int col, std::vector<int>& rows, std::vector<double>& vals) const {
    rows.clear();
    vals.clear();
    if (has_csc) {
        int start = col_ptr[col];
        int end = col_ptr[col + 1];
        for (int k = start; k < end; ++k) {
            rows.push_back(row_indices[k]);
            vals.push_back(csc_values[k]);
        }
    } else {
        for (int r = 0; r < num_rows; ++r) {
            for (int k = row_ptr[r]; k < row_ptr[r + 1]; ++k) {
                if (col_indices[k] == col) {
                    rows.push_back(r);
                    vals.push_back(values[k]);
                    break;
                }
            }
        }
    }
}

void SparseMatrix::get_row_dense(int row, std::vector<double>& row_vec) const {
    row_vec.assign(num_cols, 0.0);
    if (row >= 0 && row < num_rows) {
        int start = row_ptr[row];
        int end = row_ptr[row + 1];
        for (int k = start; k < end; ++k) {
            row_vec[col_indices[k]] = values[k];
        }
    }
}

double SparseMatrix::get_element(int r, int c) const {
    if (r < 0 || r >= num_rows || c < 0 || c >= num_cols) return 0.0;
    int start = row_ptr[r];
    int end = row_ptr[r + 1];
    for (int k = start; k < end; ++k) {
        if (col_indices[k] == c) return values[k];
    }
    return 0.0;
}

SparseMatrix SparseMatrix::transpose() const {
    std::vector<MatrixTriplet> triplets;
    triplets.reserve(values.size());
    for (int r = 0; r < num_rows; ++r) {
        for (int k = row_ptr[r]; k < row_ptr[r + 1]; ++k) {
            triplets.emplace_back(col_indices[k], r, values[k]);
        }
    }
    return SparseMatrix::from_triplets(num_cols, num_rows, triplets);
}

} // namespace hunters
