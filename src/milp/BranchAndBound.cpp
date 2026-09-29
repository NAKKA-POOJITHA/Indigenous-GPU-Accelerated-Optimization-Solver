#include "milp/BranchAndBound.hpp"
#include "presolve/Presolver.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <deque>

namespace hunters {

struct NodeCompareMin {
    bool operator()(const Node& a, const Node& b) const {
        return a.lower_bound > b.lower_bound; // Smallest bound has highest priority
    }
};

struct NodeCompareMax {
    bool operator()(const Node& a, const Node& b) const {
        return a.lower_bound < b.lower_bound; // Largest bound has highest priority
    }
};

MILPResult BranchAndBoundSolver::solve(const Model& model) {
    auto start_time = std::chrono::high_resolution_clock::now();
    MILPResult res;
    res.node_count = 0;
    res.total_lp_iterations = 0;

    bool is_min = (model.sense == ObjectiveSense::MINIMIZE);
    double incumbent = is_min ? 1e30 : -1e30;
    double global_best_bound = is_min ? -1e30 : 1e30;
    std::vector<double> incumbent_sol;
    bool has_incumbent = false;

    // 1. Solve Root LP Relaxation
    SimplexSolver root_solver(tolerances);
    SimplexResult root_lp_res = root_solver.solve(model);
    res.total_lp_iterations += root_lp_res.diagnostics.num_iterations;

    if (root_lp_res.status == SolverStatus::INFEASIBLE) {
        res.status = SolverStatus::INFEASIBLE;
        res.incumbent_objective = incumbent;
        res.best_bound = incumbent;
        res.mip_gap = 0.0;
        res.solve_time_sec = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start_time).count();
        return res;
    }

    if (root_lp_res.status == SolverStatus::UNBOUNDED) {
        res.status = SolverStatus::UNBOUNDED;
        res.solve_time_sec = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start_time).count();
        return res;
    }

    if (root_lp_res.status != SolverStatus::OPTIMAL) {
        res.status = root_lp_res.status;
        res.solve_time_sec = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start_time).count();
        return res;
    }

    global_best_bound = root_lp_res.objective_value;

    // Check if root LP solution is already integral
    int root_branch_var = BranchingRules::select_branching_variable(model, root_lp_res.primal_solution,
                                                                   tolerances.integrality_tolerance,
                                                                   branching_strategy);
    if (root_branch_var == -1) {
        // Root solution is integer feasible and optimal!
        res.status = SolverStatus::OPTIMAL;
        res.incumbent_objective = root_lp_res.objective_value;
        res.best_bound = root_lp_res.objective_value;
        res.mip_gap = 0.0;
        res.node_count = 1;
        res.best_solution = root_lp_res.primal_solution;
        res.solve_time_sec = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start_time).count();
        return res;
    }

    // 2. Run Primal Rounding Heuristic at Root Node
    if (enable_heuristics) {
        HeuristicResult heur = PrimalRoundingHeuristic::run(model, root_lp_res.primal_solution, tolerances);
        if (heur.found_feasible) {
            has_incumbent = true;
            incumbent = heur.objective;
            incumbent_sol = heur.solution;
        }
    }

    // 3. Initialize Node Container
    std::deque<Node> node_queue;
    int next_node_id = 0;

    Node root_node(next_node_id++, -1, 0, root_lp_res.objective_value);
    node_queue.push_back(root_node);

    // B&B Main Exploration Loop
    while (!node_queue.empty()) {
        res.node_count++;

        // Node selection:
        Node current_node;
        if (node_strategy == NodeSelectionStrategy::DEPTH_FIRST) {
            current_node = node_queue.back();
            node_queue.pop_back();
        } else { // Best Bound
            auto best_it = node_queue.begin();
            for (auto it = node_queue.begin(); it != node_queue.end(); ++it) {
                if (is_min) {
                    if (it->lower_bound < best_it->lower_bound) best_it = it;
                } else {
                    if (it->lower_bound > best_it->lower_bound) best_it = it;
                }
            }
            current_node = *best_it;
            node_queue.erase(best_it);
        }

        // Update global best bound across all open nodes
        if (is_min) {
            double current_min_bound = current_node.lower_bound;
            for (const auto& nd : node_queue) {
                current_min_bound = std::min(current_min_bound, nd.lower_bound);
            }
            global_best_bound = current_min_bound;
        } else {
            double current_max_bound = current_node.lower_bound;
            for (const auto& nd : node_queue) {
                current_max_bound = std::max(current_max_bound, nd.lower_bound);
            }
            global_best_bound = current_max_bound;
        }

        // Check time limit / node limit
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration<double>(now - start_time).count() > tolerances.time_limit_sec) {
            res.status = SolverStatus::TIME_LIMIT;
            break;
        }
        if (res.node_count >= tolerances.max_nodes) {
            res.status = SolverStatus::ITERATION_LIMIT;
            break;
        }

        // Bound Pruning
        if (has_incumbent) {
            if (is_min && current_node.lower_bound >= incumbent - tolerances.optimality_tolerance) {
                continue; // Pruned by bound
            } else if (!is_min && current_node.lower_bound <= incumbent + tolerances.optimality_tolerance) {
                continue; // Pruned by bound
            }
        }

        // Build child node LP model by applying node bound modifications
        Model node_model = model;
        for (const auto& lb_mod : current_node.custom_lower_bounds) {
            int v_idx = lb_mod.first;
            node_model.variables[v_idx].lower_bound = std::max(node_model.variables[v_idx].lower_bound, lb_mod.second);
        }
        for (const auto& ub_mod : current_node.custom_upper_bounds) {
            int v_idx = ub_mod.first;
            node_model.variables[v_idx].upper_bound = std::min(node_model.variables[v_idx].upper_bound, ub_mod.second);
        }

        // Solve LP relaxation for this node
        SimplexSolver node_solver(tolerances);
        SimplexResult node_lp = node_solver.solve(node_model);
        res.total_lp_iterations += node_lp.diagnostics.num_iterations;

        if (node_lp.status == SolverStatus::INFEASIBLE || node_lp.status == SolverStatus::UNBOUNDED) {
            continue; // Pruned by infeasibility / unboundedness
        }
        if (node_lp.status != SolverStatus::OPTIMAL) {
            continue;
        }

        double node_obj = node_lp.objective_value;

        // Bound check against incumbent
        if (has_incumbent) {
            if (is_min && node_obj >= incumbent - tolerances.optimality_tolerance) {
                continue; // Pruned by bound
            } else if (!is_min && node_obj <= incumbent + tolerances.optimality_tolerance) {
                continue; // Pruned by bound
            }
        }

        // Check integrality
        int branch_var = BranchingRules::select_branching_variable(node_model, node_lp.primal_solution,
                                                                   tolerances.integrality_tolerance,
                                                                   branching_strategy);

        if (branch_var == -1) {
            // Integer feasible candidate found!
            bool is_new_best = false;
            if (!has_incumbent) {
                is_new_best = true;
            } else {
                if (is_min && node_obj < incumbent - tolerances.optimality_tolerance) is_new_best = true;
                if (!is_min && node_obj > incumbent + tolerances.optimality_tolerance) is_new_best = true;
            }

            if (is_new_best) {
                has_incumbent = true;
                incumbent = node_obj;
                incumbent_sol = node_lp.primal_solution;
            }
            continue; // Pruned by integrality
        }

        // Branching: variable x[branch_var] has fractional value x*
        double x_star = node_lp.primal_solution[branch_var];
        double floor_val = std::floor(x_star);
        double ceil_val = std::ceil(x_star);

        // Left Child: x[branch_var] <= floor(x*)
        Node left_child(next_node_id++, current_node.id, current_node.depth + 1, node_obj);
        left_child.custom_lower_bounds = current_node.custom_lower_bounds;
        left_child.custom_upper_bounds = current_node.custom_upper_bounds;
        left_child.custom_upper_bounds.push_back({branch_var, floor_val});

        // Right Child: x[branch_var] >= ceil(x*)
        Node right_child(next_node_id++, current_node.id, current_node.depth + 1, node_obj);
        right_child.custom_lower_bounds = current_node.custom_lower_bounds;
        right_child.custom_upper_bounds = current_node.custom_upper_bounds;
        right_child.custom_lower_bounds.push_back({branch_var, ceil_val});

        node_queue.push_back(left_child);
        node_queue.push_back(right_child);

        // Check MIP gap
        if (has_incumbent) {
            double gap = std::abs(global_best_bound - incumbent) / std::max(1.0, std::abs(incumbent));
            if (gap <= tolerances.mip_gap_tolerance) {
                break; // Target MIP gap reached
            }
        }
    }

    if (has_incumbent) {
        res.status = SolverStatus::OPTIMAL;
        res.incumbent_objective = incumbent;
        res.best_bound = (node_queue.empty()) ? incumbent : global_best_bound;
        res.mip_gap = std::abs(res.best_bound - res.incumbent_objective) / std::max(1.0, std::abs(res.incumbent_objective));
        res.best_solution = incumbent_sol;
    } else {
        res.status = SolverStatus::INFEASIBLE;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.solve_time_sec = std::chrono::duration<double>(end_time - start_time).count();
    return res;
}

} // namespace hunters
