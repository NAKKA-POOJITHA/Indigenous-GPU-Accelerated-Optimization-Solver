#include "milp/Heuristics.hpp"
#include "numerical/SolutionValidator.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace hunters {

HeuristicResult PrimalRoundingHeuristic::run(const Model& model,
                                            const std::vector<double>& continuous_lp_solution,
                                            const SolverTolerances& tol) {
    HeuristicResult res;
    if (continuous_lp_solution.size() != model.variables.size()) {
        return res;
    }

    auto evaluate_and_update = [&](const std::vector<double>& cand) {
        double obj = model.obj_offset;
        for (size_t j = 0; j < model.variables.size(); ++j) {
            obj += model.variables[j].obj_coefficient * cand[j];
        }
        ValidationReport val_rep = SolutionValidator::validate(model, cand, obj, tol);
        if (val_rep.is_valid) {
            bool is_better = false;
            if (!res.found_feasible) {
                is_better = true;
            } else {
                if (model.sense == ObjectiveSense::MINIMIZE && obj < res.objective) is_better = true;
                if (model.sense == ObjectiveSense::MAXIMIZE && obj > res.objective) is_better = true;
            }
            if (is_better) {
                res.found_feasible = true;
                res.objective = obj;
                res.solution = cand;
            }
        }
    };

    // Strategy 1: Global FLOOR
    {
        std::vector<double> cand = continuous_lp_solution;
        for (size_t j = 0; j < model.variables.size(); ++j) {
            const auto& v = model.variables[j];
            if (v.is_integer()) {
                cand[j] = std::max(v.lower_bound, std::min(v.upper_bound, std::floor(cand[j])));
            }
        }
        evaluate_and_update(cand);
    }

    // Strategy 2: Global NEAREST
    {
        std::vector<double> cand = continuous_lp_solution;
        for (size_t j = 0; j < model.variables.size(); ++j) {
            const auto& v = model.variables[j];
            if (v.is_integer()) {
                cand[j] = std::max(v.lower_bound, std::min(v.upper_bound, std::round(cand[j])));
            }
        }
        evaluate_and_update(cand);
    }

    // Strategy 3: Greedy Objective-Sorted Sequential Rounding with Feasibility Repair
    // Sort fractional variables by objective magnitude
    std::vector<size_t> frac_indices;
    for (size_t j = 0; j < model.variables.size(); ++j) {
        const auto& v = model.variables[j];
        if (v.is_integer()) {
            double dist = std::abs(continuous_lp_solution[j] - std::round(continuous_lp_solution[j]));
            if (dist > tol.integrality_tolerance) {
                frac_indices.push_back(j);
            }
        }
    }

    // Sort by best objective direction
    bool is_max = (model.sense == ObjectiveSense::MAXIMIZE);
    std::sort(frac_indices.begin(), frac_indices.end(), [&](size_t a, size_t b) {
        if (is_max) {
            return model.variables[a].obj_coefficient > model.variables[b].obj_coefficient;
        } else {
            return model.variables[a].obj_coefficient < model.variables[b].obj_coefficient;
        }
    });

    for (size_t target_var : frac_indices) {
        std::vector<double> cand = continuous_lp_solution;
        // Floor all variables
        for (size_t j = 0; j < model.variables.size(); ++j) {
            const auto& v = model.variables[j];
            if (v.is_integer()) {
                cand[j] = std::max(v.lower_bound, std::min(v.upper_bound, std::floor(cand[j])));
            }
        }
        // Round the high-value target variable UP
        const auto& tv = model.variables[target_var];
        cand[target_var] = std::max(tv.lower_bound, std::min(tv.upper_bound, std::ceil(continuous_lp_solution[target_var])));
        evaluate_and_update(cand);
    }

    return res;
}

} // namespace hunters
