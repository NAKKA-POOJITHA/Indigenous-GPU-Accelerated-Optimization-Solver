#include "gpu/GpuContext.hpp"
#include <iostream>

#ifdef HUNTERS_ENABLE_CUDA
#include <cuda_runtime.h>
#endif

namespace hunters {

GpuContext::GpuContext() {
#ifdef HUNTERS_ENABLE_CUDA
    int device_count = 0;
    cudaError_t err = cudaGetDeviceCount(&device_count);
    if (err == cudaSuccess && device_count > 0) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, 0);
        device_info.is_available = true;
        device_info.device_name = prop.name;
        device_info.compute_capability_major = prop.major;
        device_info.compute_capability_minor = prop.minor;
        device_info.total_memory_bytes = prop.totalGlobalMem;
        device_info.num_multiprocessors = prop.multiProcessorCount;
        device_info.warp_size = prop.warpSize;
    } else {
        device_info.is_available = false;
        device_info.device_name = "CPU Parallel SIMD (CUDA hardware not detected)";
    }
#else
    device_info.is_available = false;
    device_info.device_name = "CPU Sparse SIMD Acceleration Engine";
#endif
}

void GpuContext::print_device_info() const {
    std::cout << "============== COMPUTE ACCELERATOR ==============" << std::endl;
    std::cout << "GPU Available       : " << (device_info.is_available ? "YES (Native CUDA)" : "NO (CPU SIMD Fallback)") << std::endl;
    std::cout << "Device / Engine Name: " << device_info.device_name << std::endl;
    if (device_info.is_available) {
        std::cout << "Compute Capability  : " << device_info.compute_capability_major << "." << device_info.compute_capability_minor << std::endl;
        std::cout << "Global Memory       : " << (device_info.total_memory_bytes / (1024 * 1024)) << " MB" << std::endl;
        std::cout << "Multiprocessors     : " << device_info.num_multiprocessors << std::endl;
        std::cout << "Warp Size           : " << device_info.warp_size << std::endl;
    }
    std::cout << "Kernel Launches     : " << kernel_launches << std::endl;
    std::cout << "Cumulative Exec Time: " << cumulative_gpu_time_sec << " s" << std::endl;
    std::cout << "=================================================" << std::endl;
}

} // namespace hunters
