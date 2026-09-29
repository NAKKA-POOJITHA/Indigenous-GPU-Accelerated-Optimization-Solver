#include "model/Model.hpp"
#include "api/HuntersSolver.hpp"
#include "api/LPParser.hpp"
#include "gpu/GpuSpMV.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

namespace hunters {

void test_end_to_end() {
    std::cout << "[TEST] Running test_end_to_end..." << std::endl;

    // String containing .lp problem format: Facility Location
    std::string lp_str = R"(
Maximize
  obj: 120 x_open_1 + 150 x_open_2 + 80 y_serve_1 + 90 y_serve_2
Subject To
  c_demand1: y_serve_1 <= 100
  c_demand2: y_serve_2 <= 120
  c_link1: y_serve_1 - 100 x_open_1 <= 0
  c_link2: y_serve_2 - 120 x_open_2 <= 0
  c_budget: 50 x_open_1 + 70 x_open_2 <= 100
Bounds
  0 <= y_serve_1 <= 100
  0 <= y_serve_2 <= 120
Binaries
  x_open_1 x_open_2
End
)";

    Model model = LPParser::parse_string(lp_str);
    assert(model.variables.size() >= 4);
    assert(model.constraints.size() >= 5);
    assert(model.is_milp());

    SolverOptions opt;
    opt.enable_presolve = true;
    opt.enable_scaling = true;
    opt.enable_heuristics = true;

    HuntersSolver solver(opt);
    SolverResult res = solver.solve(model);

    assert(res.status == SolverStatus::OPTIMAL);
    assert(res.validation_report.is_valid);
    assert(res.mip_gap < 1e-3);

    std::cout << "  -> test_end_to_end PASSED (Parsed, presolved, solved, and validated)" << std::endl;
}

} // namespace hunters
