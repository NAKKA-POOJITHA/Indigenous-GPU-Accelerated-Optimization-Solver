#ifndef HUNTERS_GPU_GPU_SPMV_HPP
#define HUNTERS_GPU_GPU_SPMV_HPP

#include "sparse/SparseMatrix.hpp"
#include "gpu/GpuContext.hpp"
#include <vector>

namespace hunters {

struct SpMVBenchmarkResult {
    int num_rows;
    int num_cols;
    size_t nnz;
    int num_iterations;
    double cpu_time_ms;
    double gpu_time_ms;
    double speedup;
    double cpu_gflops;
    double gpu_gflops;
    double max_discrepancy;
};

class GpuSpMV {
public:
    // Performs y = alpha * A * x + beta * y using GPU kernel (or optimized vector fallback)
    static void compute_spmv(const SparseMatrix& A,
                            const std::vector<double>& x,
                            std::vector<double>& y,
                            double alpha = 1.0,
                            double beta = 0.0);

    // Performs residual calculation r = b - A * x
    static void compute_residual(const SparseMatrix& A,
                                const std::vector<double>& x,
                                const std::vector<double>& b,
                                std::vector<double>& residual);

    // High precision benchmark comparing CPU and GPU implementations
    static SpMVBenchmarkResult benchmark(const SparseMatrix& A, int num_iterations = 100);
};

} // namespace hunters

#endif // HUNTERS_GPU_GPU_SPMV_HPP
