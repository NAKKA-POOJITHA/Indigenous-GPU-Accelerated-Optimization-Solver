#include "model/Model.hpp"
#include "api/HuntersSolver.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

namespace hunters {

void test_ill_conditioned() {
    std::cout << "[TEST] Running test_ill_conditioned..." << std::endl;

    // Badly scaled problem:
    // Min 1e-4*x1 + 1e5*x2
    // s.t. 1e-4*x1 + 1e5*x2 >= 1.0
    //      1e-3*x1 + 1e-2*x2 <= 10.0
    //      x1, x2 >= 0
    Model model("IllConditionedLP");
    model.minimize();
    int x1 = model.add_variable("x1", 0.0, 1e30, 1e-4);
    int x2 = model.add_variable("x2", 0.0, 1e30, 1e5);

    model.add_constraint("c1", {x1, x2}, {1e-4, 1e5}, ConstraintSense::GREATER_EQUAL, 1.0);
    model.add_constraint("c2", {x1, x2}, {1e-3, 1e-2}, ConstraintSense::LESS_EQUAL, 10.0);

    SolverOptions options;
    options.enable_scaling = true;
    options.enable_presolve = true;

    HuntersSolver solver(options);
    SolverResult res = solver.solve(model);

    assert(res.status == SolverStatus::OPTIMAL);
    assert(res.validation_report.is_valid);

    std::cout << "  -> test_ill_conditioned PASSED (Conditioning handled robustly)" << std::endl;
}

} // namespace hunters
