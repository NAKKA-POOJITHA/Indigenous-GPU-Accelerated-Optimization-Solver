#!/usr/bin/env python3
"""
Master Verification Script for Hunters Indigenous Optimization Solver
Problem Statement 26119: Indigenous Sovereign Alternative to CPLEX/Xpress
Runs all unit tests, integration tests, industrial cases, and benchmark suites.
"""

import os
import sys
import subprocess
import time

def run_step(step_name: str, cmd: list, cwd: str) -> bool:
    print("\n" + "=" * 80)
    print(f"[*] STEP: {step_name}")
    print(f"[*] COMMAND: {' '.join(cmd)}")
    print("=" * 80)
    t0 = time.perf_counter()
    res = subprocess.run(cmd, cwd=cwd, text=True)
    elapsed = time.perf_counter() - t0
    if res.returncode == 0:
        print(f"[+] {step_name} PASSED in {elapsed:.2f}s")
        return True
    else:
        print(f"[-] {step_name} FAILED with exit code {res.returncode}")
        return False

def main():
    workspace = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    build_dir = os.path.join(workspace, "build")
    python_dir = os.path.join(workspace, "python")

    print("=" * 80)
    print("      HUNTERS INDIGENOUS OPTIMIZATION SOLVER — FULL SYSTEM VERIFICATION       ")
    print("         Problem Statement 26119 (MRPL Sovereign CPLEX/Xpress Alternative)    ")
    print("=" * 80)

    steps = [
        ("1. C++ Mathematical Core Test Suite (9 modules)",
         [os.path.join(build_dir, "hunters_tests.exe")],
         build_dir),

        ("2. MRPL Mangalore Refinery Industrial Crude Blending MILP Case Study",
         [os.path.join(build_dir, "hunters-solver.exe"), os.path.join(workspace, "examples", "refinery", "mrpl_crude_blending.lp"), "--verbose"],
         build_dir),

        ("3. Python LP Modeling API & Solution Verification",
         [sys.executable, os.path.join(workspace, "examples", "lp", "production_example.py")],
         workspace),

        ("4. Python MILP Capital Budgeting Branch-and-Bound API",
         [sys.executable, os.path.join(workspace, "examples", "milp", "knapsack_example.py")],
         workspace),

        ("5. Comprehensive Mathematical Optimization Benchmark Suite (12 instances)",
         [sys.executable, os.path.join(workspace, "scripts", "run_benchmarks.py")],
         workspace),

        ("6. GPU / SIMD Sparse Linear Algebra Acceleration Benchmark",
         [sys.executable, os.path.join(workspace, "scripts", "benchmark_gpu.py")],
         workspace),
    ]

    all_passed = True
    for name, cmd, cwd in steps:
        ok = run_step(name, cmd, cwd)
        if not ok:
            all_passed = False
            break

    print("\n" + "=" * 80)
    if all_passed:
        print(" [SUCCESS] ALL 6 SYSTEM VERIFICATION STEPS PASSED WITH 100% INTEGRITY!")
        print(" Sovereign solver MVP is fully verified, robust, and ready for demonstration.")
    else:
        print(" [FAILURE] Some verification steps failed. Please review logs.")
    print("=" * 80 + "\n")

    sys.exit(0 if all_passed else 1)

if __name__ == "__main__":
    main()
