#include "presolve/Presolver.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

namespace hunters {

void test_presolve() {
    std::cout << "[TEST] Running test_presolve..." << std::endl;

    // Model with:
    // x1 fixed (lb=5, ub=5)
    // x2 has singleton constraint (2 * x2 <= 10 => x2 <= 5)
    // x3 normal
    // redundant empty row
    Model model("PresolveTest");
    int x1 = model.add_variable("x1", 5.0, 5.0, 10.0);
    int x2 = model.add_variable("x2", 0.0, 100.0, 20.0);
    int x3 = model.add_variable("x3", 0.0, 50.0, 30.0);

    // Constraint 1: 2*x2 <= 10 (singleton)
    model.add_constraint("c_singleton", {x2}, {2.0}, ConstraintSense::LESS_EQUAL, 10.0);
    // Constraint 2: x1 + x3 <= 25 (x1 fixed => x3 <= 20)
    model.add_constraint("c_main", {x1, x3}, {1.0, 1.0}, ConstraintSense::LESS_EQUAL, 25.0);
    // Constraint 3: empty redundant row
    model.add_constraint("c_empty", {}, {}, ConstraintSense::LESS_EQUAL, 10.0);

    Presolver presolver;
    Model reduced = presolver.presolve(model);

    assert(presolver.summary.fixed_variables >= 1);
    assert(presolver.summary.removed_constraints >= 1);
    assert(reduced.variables.size() < model.variables.size());

    // Postsolve check
    std::vector<double> red_sol(reduced.variables.size(), 2.0);
    std::vector<double> full_sol = presolver.postsolve(model, red_sol);

    assert(full_sol.size() == model.variables.size());
    assert(std::abs(full_sol[x1] - 5.0) < 1e-12); // Fixed variable restored

    std::cout << "  -> test_presolve PASSED" << std::endl;
}

} // namespace hunters
