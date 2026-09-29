#ifndef HUNTERS_GPU_GPU_CONTEXT_HPP
#define HUNTERS_GPU_GPU_CONTEXT_HPP

#include <string>
#include <iostream>

namespace hunters {

struct GpuDeviceInfo {
    bool is_available = false;
    std::string device_name = "None";
    int compute_capability_major = 0;
    int compute_capability_minor = 0;
    size_t total_memory_bytes = 0;
    int num_multiprocessors = 0;
    int warp_size = 32;
};

class GpuContext {
public:
    static GpuContext& instance() {
        static GpuContext ctx;
        return ctx;
    }

    bool is_gpu_available() const { return device_info.is_available; }
    const GpuDeviceInfo& get_device_info() const { return device_info; }

    int get_kernel_launch_count() const { return kernel_launches; }
    double get_gpu_time_sec() const { return cumulative_gpu_time_sec; }

    void record_kernel_launch(double execution_time_sec) {
        kernel_launches++;
        cumulative_gpu_time_sec += execution_time_sec;
    }

    void reset_stats() {
        kernel_launches = 0;
        cumulative_gpu_time_sec = 0.0;
    }

    void print_device_info() const;

private:
    GpuContext();
    GpuDeviceInfo device_info;
    int kernel_launches = 0;
    double cumulative_gpu_time_sec = 0.0;
};

} // namespace hunters

#endif // HUNTERS_GPU_GPU_CONTEXT_HPP
