#ifndef HUNTERS_CUDA_VECTOR_KERNEL_CU
#define HUNTERS_CUDA_VECTOR_KERNEL_CU

#ifdef __CUDACC__
#include <cuda_runtime.h>

extern "C" {

// 1. Vector AXPY Kernel: y[i] = alpha * x[i] + y[i]
__global__ void vector_axpy_kernel(int n, double alpha, const double* __restrict__ x, double* __restrict__ y) {
    int idx = blockDim.x * blockIdx.x + threadIdx.x;
    if (idx < n) {
        y[idx] += alpha * x[idx];
    }
}

// 2. Vector Dot Product Kernel with Shared Memory Reduction
__global__ void vector_dot_kernel(int n, const double* __restrict__ a, const double* __restrict__ b, double* __restrict__ block_sums) {
    extern __shared__ double sdata[];
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x * (blockDim.x * 2) + threadIdx.x;

    double my_sum = 0.0;
    if (i < n) my_sum += a[i] * b[i];
    if (i + blockDim.x < n) my_sum += a[i + blockDim.x] * b[i + blockDim.x];

    sdata[tid] = my_sum;
    __syncthreads();

    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }

    if (tid == 0) {
        block_sums[blockIdx.x] = sdata[0];
    }
}

} // extern "C"

#endif // __CUDACC__

#endif // HUNTERS_CUDA_VECTOR_KERNEL_CU
