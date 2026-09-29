#include "model/Model.hpp"
#include "milp/BranchAndBound.hpp"
#include "numerical/SolutionValidator.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

namespace hunters {

void test_branch_and_bound() {
    std::cout << "[TEST] Running test_branch_and_bound..." << std::endl;

    // Test 1: 0-1 Knapsack Problem
    // Max 10*x1 + 15*x2 + 25*x3 + 20*x4
    // s.t. 2*x1 + 3*x2 + 5*x3 + 4*x4 <= 7
    // x1, x2, x3, x4 in {0, 1}
    {
        Model model("Knapsack01");
        model.maximize();
        int x1 = model.add_variable("x1", 0.0, 1.0, 10.0, VarType::BINARY);
        int x2 = model.add_variable("x2", 0.0, 1.0, 15.0, VarType::BINARY);
        int x3 = model.add_variable("x3", 0.0, 1.0, 25.0, VarType::BINARY);
        int x4 = model.add_variable("x4", 0.0, 1.0, 20.0, VarType::BINARY);

        model.add_constraint("capacity", {x1, x2, x3, x4}, {2.0, 3.0, 5.0, 4.0}, ConstraintSense::LESS_EQUAL, 7.0);

        BranchAndBoundSolver bb;
        MILPResult res = bb.solve(model);

        std::cout << "  Knapsack incumbent: " << res.incumbent_objective << ", best_bound: " << res.best_bound << ", nodes: " << res.node_count << ", status: " << static_cast<int>(res.status) << std::endl;
        assert(res.status == SolverStatus::OPTIMAL);
        assert(std::abs(res.incumbent_objective - 35.0) < 1e-4);
        assert(res.mip_gap < 1e-3);

        ValidationReport val = SolutionValidator::validate(model, res.best_solution, res.incumbent_objective);
        assert(val.is_valid);
    }

    // Test 2: Mixed-Integer Minimization
    // Min x + 2*y
    // s.t. x + 4*y >= 7
    // x >= 0 continuous, y >= 0 integer
    {
        Model model("MixedIntMin");
        model.minimize();
        int x = model.add_variable("x", 0.0, 1e30, 1.0, VarType::CONTINUOUS);
        int y = model.add_variable("y", 0.0, 1e30, 2.0, VarType::INTEGER);

        model.add_constraint("c1", {x, y}, {1.0, 4.0}, ConstraintSense::GREATER_EQUAL, 7.0);

        BranchAndBoundSolver bb;
        MILPResult res = bb.solve(model);

        assert(res.status == SolverStatus::OPTIMAL);
        assert(std::abs(res.incumbent_objective - 4.0) < 1e-4);
        assert(std::abs(res.best_solution[y] - 2.0) < 1e-4);
        assert(std::abs(res.best_solution[x] - 0.0) < 1e-4);

        ValidationReport val = SolutionValidator::validate(model, res.best_solution, res.incumbent_objective);
        assert(val.is_valid);
    }

    std::cout << "  -> test_branch_and_bound PASSED" << std::endl;
}

} // namespace hunters
