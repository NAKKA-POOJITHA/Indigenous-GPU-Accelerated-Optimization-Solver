#include "api/HuntersSolver.hpp"
#include "api/LPParser.hpp"
#include "gpu/GpuSpMV.hpp"
#include <iostream>
#include <string>
#include <vector>

void print_help(const char* prog) {
    std::cout << "Hunters Indigenous Optimization Solver (Sovereign Alternative to CPLEX/Xpress)\n";
    std::cout << "Usage: " << prog << " <model.lp> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --solver <auto|lp|milp>           Specify problem solver type (default: auto)\n";
    std::cout << "  --method <simplex>                LP method (default: simplex)\n";
    std::cout << "  --node-selection <best-bound|dfs> MILP node selection (default: best-bound)\n";
    std::cout << "  --branching <most-fractional>     MILP branching rule (default: most-fractional)\n";
    std::cout << "  --presolve <on|off>               Enable/disable presolver (default: on)\n";
    std::cout << "  --scaling <on|off>                Enable/disable scaling (default: on)\n";
    std::cout << "  --gpu <on|off>                    Enable/disable GPU acceleration (default: off)\n";
    std::cout << "  --heuristics <on|off>             Enable/disable primal heuristics (default: on)\n";
    std::cout << "  --time-limit <sec>                Time limit in seconds (default: 300.0)\n";
    std::cout << "  --mip-gap <tol>                   MIP optimality gap tolerance (default: 0.0001)\n";
    std::cout << "  --benchmark-gpu                   Run GPU SpMV benchmark on model\n";
    std::cout << "  --verbose                         Print verbose solving steps\n";
    std::cout << "  --help                            Show this help message\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    std::string lp_filepath = "";
    hunters::SolverOptions options;
    bool run_gpu_bench = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_help(argv[0]);
            return 0;
        } else if (arg == "--solver" && i + 1 < argc) {
            std::string val = argv[++i];
            if (val == "lp") options.algorithm = hunters::SolverAlgorithm::SIMPLEX;
            else if (val == "milp") options.algorithm = hunters::SolverAlgorithm::BRANCH_AND_BOUND;
            else options.algorithm = hunters::SolverAlgorithm::AUTO;
        } else if (arg == "--method" && i + 1 < argc) {
            std::string val = argv[++i];
            // default simplex
        } else if (arg == "--node-selection" && i + 1 < argc) {
            std::string val = argv[++i];
            if (val == "dfs" || val == "depth-first") options.node_selection = hunters::NodeSelectionStrategy::DEPTH_FIRST;
            else options.node_selection = hunters::NodeSelectionStrategy::BEST_BOUND;
        } else if (arg == "--branching" && i + 1 < argc) {
            std::string val = argv[++i];
            if (val == "strong") options.branching_strategy = hunters::BranchingStrategy::STRONG_BRANCHING;
            else options.branching_strategy = hunters::BranchingStrategy::MOST_FRACTIONAL;
        } else if (arg == "--presolve" && i + 1 < argc) {
            options.enable_presolve = (std::string(argv[++i]) != "off");
        } else if (arg == "--scaling" && i + 1 < argc) {
            options.enable_scaling = (std::string(argv[++i]) != "off");
        } else if (arg == "--gpu" && i + 1 < argc) {
            options.enable_gpu = (std::string(argv[++i]) == "on");
        } else if (arg == "--heuristics" && i + 1 < argc) {
            options.enable_heuristics = (std::string(argv[++i]) != "off");
        } else if (arg == "--time-limit" && i + 1 < argc) {
            options.tolerances.time_limit_sec = std::stod(argv[++i]);
        } else if (arg == "--mip-gap" && i + 1 < argc) {
            options.tolerances.mip_gap_tolerance = std::stod(argv[++i]);
        } else if (arg == "--benchmark-gpu") {
            run_gpu_bench = true;
        } else if (arg == "--verbose") {
            options.verbose = true;
        } else if (arg[0] != '-') {
            lp_filepath = arg;
        }
    }

    if (lp_filepath.empty()) {
        std::cerr << "Error: No LP file specified.\n";
        return 1;
    }

    try {
        hunters::Model model = hunters::LPParser::parse_file(lp_filepath);

        if (run_gpu_bench) {
            std::cout << "\nRunning GPU vs CPU SpMV benchmark on model matrix...\n";
            hunters::SimplexSolver simplex;
            hunters::StandardLP std_lp = simplex.transform_to_standard_form(model);
            auto bench = hunters::GpuSpMV::benchmark(std_lp.A, 100);
            std::cout << "Matrix Dimensions : " << bench.num_rows << " x " << bench.num_cols << " (NNZ: " << bench.nnz << ")\n";
            std::cout << "CPU Time (avg)    : " << bench.cpu_time_ms << " ms (" << bench.cpu_gflops << " GFLOPs)\n";
            std::cout << "GPU/SIMD Time     : " << bench.gpu_time_ms << " ms (" << bench.gpu_gflops << " GFLOPs)\n";
            std::cout << "Measured Speedup  : " << bench.speedup << "x\n";
            std::cout << "Discrepancy       : " << bench.max_discrepancy << "\n\n";
        }

        hunters::HuntersSolver solver(options);
        hunters::SolverResult result = solver.solve(model);
        result.print_report();

        return (result.status == hunters::SolverStatus::OPTIMAL || result.status == hunters::SolverStatus::FEASIBLE) ? 0 : 2;
    } catch (const std::exception& e) {
        std::cerr << "Fatal Solver Error: " << e.what() << std::endl;
        return 3;
    }
}
