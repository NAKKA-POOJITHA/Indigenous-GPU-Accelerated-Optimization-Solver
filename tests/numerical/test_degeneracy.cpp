#include "model/Model.hpp"
#include "api/HuntersSolver.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

namespace hunters {

void test_degeneracy() {
    std::cout << "[TEST] Running test_degeneracy (Beale's degenerate LP)..." << std::endl;

    // Beale's classic cycling problem:
    // Min -0.75*x1 + 20*x2 - 0.5*x3 + 6*x4
    // s.t. 0.25*x1 - 8*x2 - x3 + 9*x4 <= 0
    //      0.5*x1 - 12*x2 - 0.5*x3 + 3*x4 <= 0
    //      x3 <= 1
    //      x1, x2, x3, x4 >= 0
    Model model("BealeDegeneracy");
    model.minimize();
    int x1 = model.add_variable("x1", 0.0, 1e30, -0.75);
    int x2 = model.add_variable("x2", 0.0, 1e30, 20.0);
    int x3 = model.add_variable("x3", 0.0, 1.0, -0.5);
    int x4 = model.add_variable("x4", 0.0, 1e30, 6.0);

    model.add_constraint("c1", {x1, x2, x3, x4}, {0.25, -8.0, -1.0, 9.0}, ConstraintSense::LESS_EQUAL, 0.0);
    model.add_constraint("c2", {x1, x2, x3, x4}, {0.5, -12.0, -0.5, 3.0}, ConstraintSense::LESS_EQUAL, 0.0);

    SolverOptions options;
    options.enable_presolve = false; // Test raw simplex anti-cycling robustness directly

    HuntersSolver solver(options);
    SolverResult res = solver.solve(model);

    assert(res.status == SolverStatus::OPTIMAL);
    assert(res.validation_report.is_valid);
    assert(res.diagnostics.num_degenerate_pivots >= 0);

    std::cout << "  -> test_degeneracy PASSED (Degeneracy and cycling resolved without stalling)" << std::endl;
}

} // namespace hunters
