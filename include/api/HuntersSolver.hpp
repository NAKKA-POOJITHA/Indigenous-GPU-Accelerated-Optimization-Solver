#ifndef HUNTERS_API_HUNTERS_SOLVER_HPP
#define HUNTERS_API_HUNTERS_SOLVER_HPP

#include "model/Model.hpp"
#include "api/SolverOptions.hpp"
#include "api/SolverResult.hpp"

namespace hunters {

class HuntersSolver {
public:
    SolverOptions options;

    HuntersSolver(const SolverOptions& opt = SolverOptions())
        : options(opt) {}

    // Solve problem from Model object
    SolverResult solve(const Model& model);

    // Solve problem from .lp file path
    SolverResult solve_file(const std::string& lp_filepath);
};

} // namespace hunters

#endif // HUNTERS_API_HUNTERS_SOLVER_HPP
