#include "api/HuntersSolver.hpp"
#include "api/LPParser.hpp"
#include "presolve/Presolver.hpp"
#include "presolve/Scaling.hpp"
#include "lp/SimplexSolver.hpp"
#include "milp/BranchAndBound.hpp"
#include "numerical/SolutionValidator.hpp"
#include "gpu/GpuContext.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>

namespace hunters {

std::string SolverResult::status_string() const {
    switch (status) {
        case SolverStatus::OPTIMAL: return "OPTIMAL / FEASIBLE";
        case SolverStatus::FEASIBLE: return "FEASIBLE";
        case SolverStatus::INFEASIBLE: return "INFEASIBLE";
        case SolverStatus::UNBOUNDED: return "UNBOUNDED";
        case SolverStatus::ITERATION_LIMIT: return "ITERATION_LIMIT";
        case SolverStatus::TIME_LIMIT: return "TIME_LIMIT";
        case SolverStatus::NUMERICAL_ERROR: return "NUMERICAL_ERROR";
        default: return "UNKNOWN";
    }
}

void SolverResult::print_report() const {
    std::cout << "\n========== SOLVER REPORT ==========" << std::endl;
    std::cout << "\nProblem:" << std::endl;
    std::cout << "Name        : " << problem_name << std::endl;
    std::cout << "Type        : " << (is_milp ? "MILP" : "LP") << std::endl;
    std::cout << "Variables   : " << num_variables << std::endl;
    std::cout << "Constraints : " << num_constraints << std::endl;
    std::cout << "Nonzeros    : " << num_nonzeros << std::endl;

    std::cout << "\nPresolve:" << std::endl;
    std::cout << "Time        : " << std::fixed << std::setprecision(4) << presolve_time_sec << " s" << std::endl;
    if (presolve_summary.orig_variables > 0) {
        std::cout << "Removed vars: " << presolve_summary.removed_variables << std::endl;
        std::cout << "Removed rows: " << presolve_summary.removed_constraints << std::endl;
        std::cout << "Fixed vars  : " << presolve_summary.fixed_variables << std::endl;
        std::cout << "Tight bounds: " << presolve_summary.tightened_bounds << std::endl;
    }

    if (!is_milp) {
        std::cout << "\nLP (Revised Simplex):" << std::endl;
        std::cout << "Iterations  : " << lp_iterations << std::endl;
        std::cout << "Objective   : " << std::fixed << std::setprecision(6) << objective_value << std::endl;
        std::cout << "Time        : " << std::fixed << std::setprecision(4) << lp_time_sec << " s" << std::endl;
    } else {
        std::cout << "\nMILP (Branch & Bound):" << std::endl;
        std::cout << "Nodes       : " << milp_nodes << std::endl;
        std::cout << "LP Iteration: " << lp_iterations << std::endl;
        std::cout << "Incumbent   : " << std::fixed << std::setprecision(6) << objective_value << std::endl;
        std::cout << "Best Bound  : " << std::fixed << std::setprecision(6) << best_bound << std::endl;
        std::cout << "MIP Gap     : " << std::fixed << std::setprecision(4) << (mip_gap * 100.0) << "%" << std::endl;
        std::cout << "Tree Time   : " << std::fixed << std::setprecision(4) << milp_time_sec << " s" << std::endl;
    }

    std::cout << "\nGPU Acceleration:" << std::endl;
    std::cout << "Kernel calls: " << gpu_kernel_calls << std::endl;
    std::cout << "GPU time    : " << std::fixed << std::setprecision(4) << gpu_time_sec << " s" << std::endl;

    std::cout << "\nSolution Status: " << status_string() << std::endl;
    std::cout << "Total solve time: " << std::fixed << std::setprecision(4) << total_solve_time_sec << " s" << std::endl;
    std::cout << "====================================\n" << std::endl;

    validation_report.print_report();
    diagnostics.print_diagnostics();
}

SolverResult HuntersSolver::solve(const Model& model) {
    auto start_time = std::chrono::high_resolution_clock::now();
    GpuContext::instance().reset_stats();

    SolverResult res;
    res.problem_name = model.name;
    res.is_milp = model.is_milp();
    res.num_variables = model.variables.size();
    res.num_constraints = model.constraints.size();
    res.num_nonzeros = model.num_nonzeros();
    res.presolve_time_sec = 0.0;
    res.lp_time_sec = 0.0;
    res.milp_time_sec = 0.0;
    res.lp_iterations = 0;
    res.milp_nodes = 0;

    if (options.verbose) {
        std::cout << "Hunters Indigenous Optimization Solver v0.1 MVP" << std::endl;
        std::cout << "Problem Type: " << (res.is_milp ? "MILP" : "LP") << std::endl;
        std::cout << "Variables: " << res.num_variables << ", Constraints: " << res.num_constraints
                  << ", Nonzeros: " << res.num_nonzeros << std::endl;
        std::cout << "Presolve: " << (options.enable_presolve ? "ENABLED" : "DISABLED") << std::endl;
        std::cout << "Scaling: " << (options.enable_scaling ? "ENABLED" : "DISABLED") << std::endl;
        std::cout << "GPU Acceleration: " << (options.enable_gpu ? "ENABLED" : "DISABLED") << std::endl;
        std::cout << "Solving..." << std::endl;
    }

    // 1. Presolve Phase
    Model working_model = model;
    Presolver presolver;
    if (options.enable_presolve) {
        working_model = presolver.presolve(model);
        res.presolve_summary = presolver.summary;
        res.presolve_time_sec = presolver.summary.presolve_time_sec;

        if (presolver.summary.is_infeasible) {
            res.status = SolverStatus::INFEASIBLE;
            res.total_solve_time_sec = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start_time).count();
            return res;
        }
    }

    // 2. Solve LP or MILP
    if (!working_model.is_milp() && options.algorithm != SolverAlgorithm::BRANCH_AND_BOUND) {
        // LP Solve
        auto lp_start = std::chrono::high_resolution_clock::now();
        SimplexSolver simplex(options.tolerances, options.pricing_strategy);
        SimplexResult lp_res = simplex.solve(working_model);
        auto lp_end = std::chrono::high_resolution_clock::now();

        res.status = lp_res.status;
        res.objective_value = lp_res.objective_value;
        res.best_bound = lp_res.objective_value;
        res.mip_gap = 0.0;
        res.lp_iterations = lp_res.diagnostics.num_iterations;
        res.lp_time_sec = std::chrono::duration<double>(lp_end - lp_start).count();
        res.diagnostics = lp_res.diagnostics;

        if (lp_res.status == SolverStatus::OPTIMAL || lp_res.status == SolverStatus::FEASIBLE) {
            if (options.enable_presolve) {
                res.variable_values = presolver.postsolve(model, lp_res.primal_solution);
            } else {
                res.variable_values = lp_res.primal_solution;
            }
        }
    } else {
        // MILP Solve
        auto milp_start = std::chrono::high_resolution_clock::now();
        BranchAndBoundSolver bb(options.tolerances, options.node_selection, options.branching_strategy);
        bb.enable_heuristics = options.enable_heuristics;
        MILPResult milp_res = bb.solve(working_model);
        auto milp_end = std::chrono::high_resolution_clock::now();

        res.status = milp_res.status;
        res.objective_value = milp_res.incumbent_objective;
        res.best_bound = milp_res.best_bound;
        res.mip_gap = milp_res.mip_gap;
        res.milp_nodes = milp_res.node_count;
        res.lp_iterations = milp_res.total_lp_iterations;
        res.milp_time_sec = std::chrono::duration<double>(milp_end - milp_start).count();
        res.diagnostics = milp_res.diagnostics;

        if (milp_res.status == SolverStatus::OPTIMAL || milp_res.status == SolverStatus::FEASIBLE) {
            if (options.enable_presolve) {
                res.variable_values = presolver.postsolve(model, milp_res.best_solution);
            } else {
                res.variable_values = milp_res.best_solution;
            }
        }
    }

    // 3. Map variable values by name
    for (size_t j = 0; j < model.variables.size(); ++j) {
        double val = (j < res.variable_values.size()) ? res.variable_values[j] : 0.0;
        res.solution_map[model.variables[j].name] = val;
    }

    // 4. Independent Solution Validation
    if (res.status == SolverStatus::OPTIMAL || res.status == SolverStatus::FEASIBLE) {
        res.validation_report = SolutionValidator::validate(model, res.variable_values, res.objective_value, options.tolerances);
    }

    // 5. GPU Statistics
    res.gpu_kernel_calls = GpuContext::instance().get_kernel_launch_count();
    res.gpu_time_sec = GpuContext::instance().get_gpu_time_sec();

    auto end_time = std::chrono::high_resolution_clock::now();
    res.total_solve_time_sec = std::chrono::duration<double>(end_time - start_time).count();

    return res;
}

SolverResult HuntersSolver::solve_file(const std::string& lp_filepath) {
    Model model = LPParser::parse_file(lp_filepath);
    return solve(model);
}

} // namespace hunters
