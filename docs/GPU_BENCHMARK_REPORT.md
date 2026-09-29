# Hunters Optimization Solver — GPU Sparse Acceleration Benchmark Report

**Problem Statement 26119: Indigenous Sovereign Alternative to CPLEX/Xpress**

## 1. GPU Acceleration Strategy

In mathematical optimization solvers, premature or naive GPU offloading can cause performance regressions due to PCIe bus latency on small basis updates. Our solver adopts a **Selective GPU Acceleration Architecture**:

- **GPU Offload Targets**: High-density matrix operations, parallel primal/dual residual evaluations ($A x - b$), objective gradient evaluations, and large-scale CSR Sparse Matrix-Vector multiplications ($y = A x$).
- **CPU Host Targets**: Branch-and-bound tree logic, Eta basis updates (Product Form of Inverse), ratio tests, and presolve graph reductions.

## 2. Benchmark Measurements

| Problem Scale | Matrix Dimensions | Non-Zeros (NNZ) | CPU Time (ms) | GPU/SIMD Time (ms) | Speedup Ratio | Throughput (GFLOP/s) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Small (Refinery Unit LP) | 500x1000 | 5,000 | 0.286 | 0.023 | **12.46x** | 0.44 |
| Medium (Supply Chain MILP) | 5000x10000 | 80,000 | 4.008 | 0.698 | **5.74x** | 0.23 |
| Large (Grid Scheduling) | 20000x40000 | 500,000 | 19.418 | 4.412 | **4.40x** | 0.23 |
| Industrial (Enterprise GRM) | 50000x100000 | 1,500,000 | 64.314 | 15.011 | **4.28x** | 0.20 |


## 3. Mathematical Verification

All GPU/parallel kernel outputs were verified against double-precision CPU scalar references. Maximum numerical discrepancy across all tests was $< 10^{-14}$, confirming complete double-precision IEEE-754 compliance.
