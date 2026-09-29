#ifndef HUNTERS_CUDA_SPMV_KERNEL_CU
#define HUNTERS_CUDA_SPMV_KERNEL_CU

#ifdef __CUDACC__
#include <cuda_runtime.h>

extern "C" {

// 1. Scalar CSR SpMV Kernel (1 thread per row)
__global__ void spmv_csr_scalar_kernel(int num_rows,
                                       const int* __restrict__ row_ptr,
                                       const int* __restrict__ col_indices,
                                       const double* __restrict__ values,
                                       const double* __restrict__ x,
                                       double* __restrict__ y,
                                       double alpha,
                                       double beta) {
    int row = blockDim.x * blockIdx.x + threadIdx.x;
    if (row < num_rows) {
        double sum = 0.0;
        int start = row_ptr[row];
        int end = row_ptr[row + 1];
        for (int k = start; k < end; ++k) {
            sum += values[k] * x[col_indices[k]];
        }
        if (beta == 0.0) {
            y[row] = alpha * sum;
        } else {
            y[row] = alpha * sum + beta * y[row];
        }
    }
}

// 2. Vector CSR SpMV Kernel (1 warp = 32 threads per row with warp-level shuffle reduction)
__global__ void spmv_csr_vector_kernel(int num_rows,
                                       const int* __restrict__ row_ptr,
                                       const int* __restrict__ col_indices,
                                       const double* __restrict__ values,
                                       const double* __restrict__ x,
                                       double* __restrict__ y,
                                       double alpha,
                                       double beta) {
    int warp_id = (blockDim.x * blockIdx.x + threadIdx.x) >> 5; // / 32
    int lane_id = threadIdx.x & 31; // % 32

    if (warp_id < num_rows) {
        int start = row_ptr[warp_id];
        int end = row_ptr[warp_id + 1];
        double sum = 0.0;

        for (int k = start + lane_id; k < end; k += 32) {
            sum += values[k] * x[col_indices[k]];
        }

        // Warp shuffle reduction
        for (int offset = 16; offset > 0; offset /= 2) {
            sum += __shfl_down_sync(0xffffffff, sum, offset);
        }

        if (lane_id == 0) {
            if (beta == 0.0) {
                y[warp_id] = alpha * sum;
            } else {
                y[warp_id] = alpha * sum + beta * y[warp_id];
            }
        }
    }
}

// 3. Fused SpMV + Residual Kernel: r[i] = b[i] - alpha * (A * x)[i]
__global__ void spmv_csr_residual_kernel(int num_rows,
                                         const int* __restrict__ row_ptr,
                                         const int* __restrict__ col_indices,
                                         const double* __restrict__ values,
                                         const double* __restrict__ x,
                                         const double* __restrict__ b,
                                         double* __restrict__ residual) {
    int row = blockDim.x * blockIdx.x + threadIdx.x;
    if (row < num_rows) {
        double sum = 0.0;
        int start = row_ptr[row];
        int end = row_ptr[row + 1];
        for (int k = start; k < end; ++k) {
            sum += values[k] * x[col_indices[k]];
        }
        residual[row] = b[row] - sum;
    }
}

} // extern "C"

#endif // __CUDACC__

#endif // HUNTERS_CUDA_SPMV_KERNEL_CU
