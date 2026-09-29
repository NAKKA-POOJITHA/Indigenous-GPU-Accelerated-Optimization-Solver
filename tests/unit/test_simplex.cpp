#include "model/Model.hpp"
#include "lp/SimplexSolver.hpp"
#include "numerical/SolutionValidator.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

namespace hunters {

void test_simplex() {
    std::cout << "[TEST] Running test_simplex..." << std::endl;

    // Test 1: Standard 2D Maximization LP
    // Max 3*x1 + 2*x2
    // s.t. x1 + 2*x2 <= 6
    //      2*x1 + x2 <= 8
    //      x1, x2 >= 0
    {
        Model model("SimpleMaxLP");
        model.maximize();
        int x1 = model.add_variable("x1", 0.0, 1e30, 3.0);
        int x2 = model.add_variable("x2", 0.0, 1e30, 2.0);

        model.add_constraint("c1", {x1, x2}, {1.0, 2.0}, ConstraintSense::LESS_EQUAL, 6.0);
        model.add_constraint("c2", {x1, x2}, {2.0, 1.0}, ConstraintSense::LESS_EQUAL, 8.0);

        SimplexSolver solver;
        SimplexResult res = solver.solve(model);

        assert(res.status == SolverStatus::OPTIMAL);
        double expected_obj = 3.0 * (10.0 / 3.0) + 2.0 * (4.0 / 3.0); // 12.666667
        assert(std::abs(res.objective_value - expected_obj) < 1e-4);
        assert(std::abs(res.primal_solution[x1] - (10.0 / 3.0)) < 1e-4);
        assert(std::abs(res.primal_solution[x2] - (4.0 / 3.0)) < 1e-4);

        ValidationReport val = SolutionValidator::validate(model, res.primal_solution, res.objective_value);
        assert(val.is_valid);
    }

    // Test 2: Phase I Diet Problem with >= constraints
    // Min 2*x1 + 3*x2
    // s.t. x1 + x2 >= 10
    //      2*x1 + x2 >= 12
    //      x1, x2 >= 0
    {
        Model model("PhaseIDietLP");
        model.minimize();
        int x1 = model.add_variable("x1", 0.0, 1e30, 2.0);
        int x2 = model.add_variable("x2", 0.0, 1e30, 3.0);

        model.add_constraint("c1", {x1, x2}, {1.0, 1.0}, ConstraintSense::GREATER_EQUAL, 10.0);
        model.add_constraint("c2", {x1, x2}, {2.0, 1.0}, ConstraintSense::GREATER_EQUAL, 12.0);

        SimplexSolver solver;
        SimplexResult res = solver.solve(model);

        assert(res.status == SolverStatus::OPTIMAL);
        assert(std::abs(res.objective_value - 20.0) < 1e-4);
        assert(std::abs(res.primal_solution[x1] - 10.0) < 1e-4);
        assert(std::abs(res.primal_solution[x2] - 0.0) < 1e-4);

        ValidationReport val = SolutionValidator::validate(model, res.primal_solution, res.objective_value);
        assert(val.is_valid);
    }

    // Test 3: Infeasible LP
    // x1 + x2 <= 2
    // x1 + x2 >= 5
    {
        Model model("InfeasibleLP");
        model.minimize();
        int x1 = model.add_variable("x1", 0.0, 1e30, 1.0);
        int x2 = model.add_variable("x2", 0.0, 1e30, 1.0);

        model.add_constraint("c1", {x1, x2}, {1.0, 1.0}, ConstraintSense::LESS_EQUAL, 2.0);
        model.add_constraint("c2", {x1, x2}, {1.0, 1.0}, ConstraintSense::GREATER_EQUAL, 5.0);

        SimplexSolver solver;
        SimplexResult res = solver.solve(model);
        assert(res.status == SolverStatus::INFEASIBLE);
    }

    // Test 4: Unbounded LP
    // Max x1 + x2, x1 - x2 <= 2, x1, x2 >= 0
    {
        Model model("UnboundedLP");
        model.maximize();
        int x1 = model.add_variable("x1", 0.0, 1e30, 1.0);
        int x2 = model.add_variable("x2", 0.0, 1e30, 1.0);
        model.add_constraint("c1", {x1, x2}, {1.0, -1.0}, ConstraintSense::LESS_EQUAL, 2.0);

        SimplexSolver solver;
        SimplexResult res = solver.solve(model);
        assert(res.status == SolverStatus::UNBOUNDED);
    }

    std::cout << "  -> test_simplex PASSED" << std::endl;
}

} // namespace hunters
