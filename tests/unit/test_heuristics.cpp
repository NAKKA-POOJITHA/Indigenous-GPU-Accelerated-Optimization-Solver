#include "model/Model.hpp"
#include "milp/Heuristics.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

namespace hunters {

void test_heuristics() {
    std::cout << "[TEST] Running test_heuristics..." << std::endl;

    Model model("HeuristicTest");
    model.maximize();
    int x1 = model.add_variable("x1", 0.0, 1.0, 10.0, VarType::BINARY);
    int x2 = model.add_variable("x2", 0.0, 1.0, 20.0, VarType::BINARY);
    model.add_constraint("c1", {x1, x2}, {1.0, 1.0}, ConstraintSense::LESS_EQUAL, 1.0);

    // Fractional LP solution [0.5, 0.5]
    std::vector<double> lp_sol = {0.5, 0.5};
    SolverTolerances tol;

    HeuristicResult heur = PrimalRoundingHeuristic::run(model, lp_sol, tol);
    assert(heur.found_feasible);
    // Should round to [0, 1] with objective 20.0 or [1, 0] with objective 10.0 (picks 20.0 for max)
    assert(heur.objective >= 10.0);

    std::cout << "  -> test_heuristics PASSED" << std::endl;
}

} // namespace hunters
