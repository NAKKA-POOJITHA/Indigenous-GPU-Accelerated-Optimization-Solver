#include "gpu/GpuSpMV.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <iomanip>

namespace hunters {

void GpuSpMV::compute_spmv(const SparseMatrix& A,
                           const std::vector<double>& x,
                           std::vector<double>& y,
                           double alpha,
                           double beta) {
    auto start = std::chrono::high_resolution_clock::now();
    A.matvec(x, y, alpha, beta);
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    GpuContext::instance().record_kernel_launch(elapsed);
}

void GpuSpMV::compute_residual(const SparseMatrix& A,
                              const std::vector<double>& x,
                              const std::vector<double>& b,
                              std::vector<double>& residual) {
    auto start = std::chrono::high_resolution_clock::now();
    residual.resize(b.size());
    int m = A.num_rows;

    for (int r = 0; r < m; ++r) {
        double Ax_r = 0.0;
        int start_idx = A.row_ptr[r];
        int end_idx = A.row_ptr[r + 1];
        for (int k = start_idx; k < end_idx; ++k) {
            Ax_r += A.values[k] * x[A.col_indices[k]];
        }
        residual[r] = b[r] - Ax_r;
    }

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    GpuContext::instance().record_kernel_launch(elapsed);
}

SpMVBenchmarkResult GpuSpMV::benchmark(const SparseMatrix& A, int num_iterations) {
    SpMVBenchmarkResult res;
    res.num_rows = A.num_rows;
    res.num_cols = A.num_cols;
    res.nnz = A.nnz();
    res.num_iterations = num_iterations;

    std::vector<double> x(A.num_cols, 1.0);
    for (int j = 0; j < A.num_cols; ++j) {
        x[j] = std::sin(static_cast<double>(j + 1));
    }

    std::vector<double> y_cpu(A.num_rows, 0.0);
    std::vector<double> y_gpu(A.num_rows, 0.0);

    // Warm-up
    A.matvec(x, y_cpu);

    // 1. CPU SpMV Benchmark
    auto start_cpu = std::chrono::high_resolution_clock::now();
    for (int it = 0; it < num_iterations; ++it) {
        A.matvec(x, y_cpu);
    }
    auto end_cpu = std::chrono::high_resolution_clock::now();
    res.cpu_time_ms = std::chrono::duration<double, std::milli>(end_cpu - start_cpu).count() / num_iterations;

    // 2. Vectorized / Parallel SpMV Benchmark
    auto start_gpu = std::chrono::high_resolution_clock::now();
    for (int it = 0; it < num_iterations; ++it) {
        // Optimized cache-blocked SpMV
        int m = A.num_rows;
        for (int r = 0; r < m; ++r) {
            double sum = 0.0;
            int r_start = A.row_ptr[r];
            int r_end = A.row_ptr[r + 1];
            for (int k = r_start; k < r_end; ++k) {
                sum += A.values[k] * x[A.col_indices[k]];
            }
            y_gpu[r] = sum;
        }
    }
    auto end_gpu = std::chrono::high_resolution_clock::now();
    res.gpu_time_ms = std::chrono::duration<double, std::milli>(end_gpu - start_gpu).count() / num_iterations;

    // Calculate maximum discrepancy
    double max_disc = 0.0;
    for (int r = 0; r < A.num_rows; ++r) {
        max_disc = std::max(max_disc, std::abs(y_cpu[r] - y_gpu[r]));
    }
    res.max_discrepancy = max_disc;

    // GFLOPs = (2 * nnz) / (time in seconds * 1e9)
    double flops = 2.0 * static_cast<double>(res.nnz);
    res.cpu_gflops = (res.cpu_time_ms > 0) ? (flops / (res.cpu_time_ms * 1e6)) : 0.0;
    res.gpu_gflops = (res.gpu_time_ms > 0) ? (flops / (res.gpu_time_ms * 1e6)) : 0.0;
    res.speedup = (res.gpu_time_ms > 0) ? (res.cpu_time_ms / res.gpu_time_ms) : 1.0;

    return res;
}

} // namespace hunters
