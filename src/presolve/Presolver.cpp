#include "presolve/Presolver.hpp"
#include <chrono>
#include <cmath>
#include <iostream>

namespace hunters {

Model Presolver::presolve(const Model& orig_model) {
    auto start_time = std::chrono::high_resolution_clock::now();

    summary.orig_variables = orig_model.variables.size();
    summary.orig_constraints = orig_model.constraints.size();
    summary.orig_nonzeros = orig_model.num_nonzeros();

    Model current = orig_model;
    fixed_history.clear();

    bool changed = true;
    int max_passes = 10;
    int pass = 0;

    while (changed && pass < max_passes) {
        changed = false;
        pass++;

        // 1. Process Singleton Rows (Bound Tightening)
        for (auto& c : current.constraints) {
            if (c.id == -2 || c.var_indices.size() != 1) continue;

            int var_idx = c.var_indices[0];
            double coef = c.coefficients[0];
            double rhs = c.rhs;

            if (std::abs(coef) < 1e-15) continue;

            double bound_val = rhs / coef;
            auto& v = current.variables[var_idx];

            if (c.sense == ConstraintSense::EQUAL) {
                v.lower_bound = std::max(v.lower_bound, bound_val);
                v.upper_bound = std::min(v.upper_bound, bound_val);
                summary.tightened_bounds++;
                changed = true;
            } else if (c.sense == ConstraintSense::LESS_EQUAL) {
                if (coef > 0) {
                    if (bound_val < v.upper_bound) {
                        v.upper_bound = bound_val;
                        summary.tightened_bounds++;
                        changed = true;
                    }
                } else {
                    if (bound_val > v.lower_bound) {
                        v.lower_bound = bound_val;
                        summary.tightened_bounds++;
                        changed = true;
                    }
                }
            } else if (c.sense == ConstraintSense::GREATER_EQUAL) {
                if (coef > 0) {
                    if (bound_val > v.lower_bound) {
                        v.lower_bound = bound_val;
                        summary.tightened_bounds++;
                        changed = true;
                    }
                } else {
                    if (bound_val < v.upper_bound) {
                        v.upper_bound = bound_val;
                        summary.tightened_bounds++;
                        changed = true;
                    }
                }
            }

            if (v.lower_bound > v.upper_bound + 1e-8) {
                summary.is_infeasible = true;
                auto end_time = std::chrono::high_resolution_clock::now();
                summary.presolve_time_sec = std::chrono::duration<double>(end_time - start_time).count();
                return current;
            }

            c.id = -2;
            summary.removed_constraints++;
            changed = true;
        }

        // 2. Fixed Variables Detection and RHS adjustment
        for (size_t j = 0; j < current.variables.size(); ++j) {
            auto& v = current.variables[j];
            if (v.is_fixed() && v.id >= 0) {
                double fixed_val = v.lower_bound;
                fixed_history.push_back({static_cast<int>(j), fixed_val});
                summary.fixed_variables++;
                changed = true;

                for (auto& c : current.constraints) {
                    if (c.id == -2) continue;
                    for (size_t k = 0; k < c.var_indices.size(); ++k) {
                        if (c.var_indices[k] == static_cast<int>(j)) {
                            c.rhs -= c.coefficients[k] * fixed_val;
                            c.var_indices.erase(c.var_indices.begin() + k);
                            c.coefficients.erase(c.coefficients.begin() + k);
                            break;
                        }
                    }
                }
                current.obj_offset += v.obj_coefficient * fixed_val;
                v.id = -2; // Mark as removed
            }
        }

        // 3. Remove Empty / Redundant Rows
        for (size_t i = 0; i < current.constraints.size(); ++i) {
            auto& c = current.constraints[i];
            if (c.id != -2 && c.var_indices.empty()) {
                if (c.sense == ConstraintSense::LESS_EQUAL && c.rhs < -1e-8) {
                    summary.is_infeasible = true;
                    auto end_time = std::chrono::high_resolution_clock::now();
                    summary.presolve_time_sec = std::chrono::duration<double>(end_time - start_time).count();
                    return current;
                } else if (c.sense == ConstraintSense::GREATER_EQUAL && c.rhs > 1e-8) {
                    summary.is_infeasible = true;
                    auto end_time = std::chrono::high_resolution_clock::now();
                    summary.presolve_time_sec = std::chrono::duration<double>(end_time - start_time).count();
                    return current;
                } else if (c.sense == ConstraintSense::EQUAL && std::abs(c.rhs) > 1e-8) {
                    summary.is_infeasible = true;
                    auto end_time = std::chrono::high_resolution_clock::now();
                    summary.presolve_time_sec = std::chrono::duration<double>(end_time - start_time).count();
                    return current;
                }
                c.id = -2; // Mark as removed
                summary.removed_constraints++;
                changed = true;
            }
        }
    }

    // Build the Reduced Model
    Model reduced_model(orig_model.name + "_presolved");
    reduced_model.sense = orig_model.sense;
    reduced_model.obj_offset = current.obj_offset;

    orig_to_reduced_var_map.assign(orig_model.variables.size(), -1);
    reduced_to_orig_var_map.clear();

    for (size_t j = 0; j < current.variables.size(); ++j) {
        const auto& v = current.variables[j];
        if (v.id != -2) {
            int new_idx = reduced_model.add_variable(v.name, v.lower_bound, v.upper_bound, v.obj_coefficient, v.type);
            orig_to_reduced_var_map[j] = new_idx;
            reduced_to_orig_var_map.push_back(static_cast<int>(j));
        } else {
            summary.removed_variables++;
        }
    }

    for (const auto& c : current.constraints) {
        if (c.id != -2 && !c.var_indices.empty()) {
            std::vector<int> new_vars;
            std::vector<double> new_coefs;
            for (size_t k = 0; k < c.var_indices.size(); ++k) {
                int old_v = c.var_indices[k];
                int new_v = orig_to_reduced_var_map[old_v];
                if (new_v >= 0) {
                    new_vars.push_back(new_v);
                    new_coefs.push_back(c.coefficients[k]);
                }
            }
            if (!new_vars.empty()) {
                reduced_model.add_constraint(c.name, new_vars, new_coefs, c.sense, c.rhs);
            } else {
                summary.removed_constraints++;
            }
        }
    }

    summary.reduced_variables = reduced_model.variables.size();
    summary.reduced_constraints = reduced_model.constraints.size();
    summary.reduced_nonzeros = reduced_model.num_nonzeros();

    auto end_time = std::chrono::high_resolution_clock::now();
    summary.presolve_time_sec = std::chrono::duration<double>(end_time - start_time).count();

    return reduced_model;
}

std::vector<double> Presolver::postsolve(const Model& orig_model, const std::vector<double>& reduced_solution) const {
    std::vector<double> full_solution(orig_model.variables.size(), 0.0);

    // Map back reduced variables
    for (size_t new_idx = 0; new_idx < reduced_solution.size(); ++new_idx) {
        if (new_idx < reduced_to_orig_var_map.size()) {
            int orig_idx = reduced_to_orig_var_map[new_idx];
            full_solution[orig_idx] = reduced_solution[new_idx];
        }
    }

    // Fill in fixed variables
    for (const auto& record : fixed_history) {
        if (record.orig_var_idx >= 0 && record.orig_var_idx < static_cast<int>(full_solution.size())) {
            full_solution[record.orig_var_idx] = record.fixed_val;
        }
    }

    return full_solution;
}

} // namespace hunters
