#ifndef HUNTERS_NUMERICAL_TOLERANCES_HPP
#define HUNTERS_NUMERICAL_TOLERANCES_HPP

namespace hunters {

struct SolverTolerances {
    double primal_feasibility_tolerance;
    double dual_feasibility_tolerance;
    double optimality_tolerance;
    double integrality_tolerance;
    double pivot_tolerance;
    double condition_threshold_warning;
    int max_iterations;
    int max_nodes;
    double time_limit_sec;
    double mip_gap_tolerance;

    SolverTolerances()
        : primal_feasibility_tolerance(1e-7),
          dual_feasibility_tolerance(1e-7),
          optimality_tolerance(1e-7),
          integrality_tolerance(1e-5),
          pivot_tolerance(1e-10),
          condition_threshold_warning(1e9),
          max_iterations(200000),
          max_nodes(50000),
          time_limit_sec(300.0),
          mip_gap_tolerance(1e-4) {} // 0.01%
};

} // namespace hunters

#endif // HUNTERS_NUMERICAL_TOLERANCES_HPP
