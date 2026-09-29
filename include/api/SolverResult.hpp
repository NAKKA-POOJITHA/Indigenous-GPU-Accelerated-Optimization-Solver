#ifndef HUNTERS_API_SOLVER_RESULT_HPP
#define HUNTERS_API_SOLVER_RESULT_HPP

#include "lp/SimplexSolver.hpp"
#include "presolve/Presolver.hpp"
#include "numerical/SolutionValidator.hpp"
#include <string>
#include <vector>
#include <map>

namespace hunters {

struct SolverResult {
    std::string problem_name;
    bool is_milp;
    SolverStatus status;
    double objective_value;
    double best_bound;
    double mip_gap;

    size_t num_variables;
    size_t num_constraints;
    size_t num_nonzeros;

    PresolveSummary presolve_summary;
    double presolve_time_sec;

    int lp_iterations;
    double lp_time_sec;

    int milp_nodes;
    double milp_time_sec;

    int gpu_kernel_calls;
    double gpu_time_sec;

    double total_solve_time_sec;

    std::vector<double> variable_values;
    std::map<std::string, double> solution_map;

    ValidationReport validation_report;
    SolverDiagnostics diagnostics;

    void print_report() const;
    std::string status_string() const;
};

} // namespace hunters

#endif // HUNTERS_API_SOLVER_RESULT_HPP
