#include "lp/SimplexSolver.hpp"
#include "sparse/VectorOps.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace hunters {

StandardLP SimplexSolver::transform_to_standard_form(const Model& model) const {
    StandardLP std_lp;
    std_lp.orig_sense = model.sense;
    std_lp.obj_offset = model.obj_offset;

    int n_orig = static_cast<int>(model.variables.size());
    int m_orig = static_cast<int>(model.constraints.size());

    std_lp.var_shift.assign(n_orig, 0.0);
    std_lp.orig_var_map.clear();

    // Map original variables
    std::vector<int> model_var_to_std_idx(n_orig, -1);
    for (int j = 0; j < n_orig; ++j) {
        const auto& v = model.variables[j];
        double lb = (v.lower_bound > -1e20) ? v.lower_bound : 0.0;
        double ub = (v.upper_bound < 1e20) ? v.upper_bound : 1e30;

        std_lp.var_shift[j] = lb;
        double shifted_ub = (ub < 1e20) ? (ub - lb) : 1e30;

        int std_idx = static_cast<int>(std_lp.c.size());
        model_var_to_std_idx[j] = std_idx;

        // Objective coefficient (convert to MINIMIZATION)
        double c_val = (model.sense == ObjectiveSense::MAXIMIZE) ? -v.obj_coefficient : v.obj_coefficient;
        std_lp.c.push_back(c_val);
        std_lp.lb.push_back(0.0);
        std_lp.ub.push_back(1e30);
        std_lp.orig_var_map.push_back(j);

        std_lp.obj_offset += c_val * lb;
    }

    // Constraints & Slacks
    std::vector<MatrixTriplet> triplets;
    std_lp.b.assign(m_orig, 0.0);

    for (int i = 0; i < m_orig; ++i) {
        const auto& c = model.constraints[i];
        double rhs = c.rhs;

        // Adjust RHS for shifted variables
        for (size_t k = 0; k < c.var_indices.size(); ++k) {
            int orig_v = c.var_indices[k];
            double coef = c.coefficients[k];
            rhs -= coef * std_lp.var_shift[orig_v];
            triplets.emplace_back(i, model_var_to_std_idx[orig_v], coef);
        }

        // Add slack or surplus
        if (c.sense == ConstraintSense::LESS_EQUAL) {
            int slack_idx = static_cast<int>(std_lp.c.size());
            std_lp.c.push_back(0.0);
            std_lp.lb.push_back(0.0);
            std_lp.ub.push_back(1e30);
            std_lp.orig_var_map.push_back(-1); // Slack
            triplets.emplace_back(i, slack_idx, 1.0);
            std_lp.slack_rows.push_back(slack_idx);
        } else if (c.sense == ConstraintSense::GREATER_EQUAL) {
            int surplus_idx = static_cast<int>(std_lp.c.size());
            std_lp.c.push_back(0.0);
            std_lp.lb.push_back(0.0);
            std_lp.ub.push_back(1e30);
            std_lp.orig_var_map.push_back(-1); // Surplus
            triplets.emplace_back(i, surplus_idx, -1.0);
            std_lp.slack_rows.push_back(surplus_idx);
        }

        std_lp.b[i] = rhs;
    }

    // Add explicit upper bound constraints for bounded variables
    int m_total = m_orig;
    for (int j = 0; j < n_orig; ++j) {
        const auto& v = model.variables[j];
        if (v.upper_bound < 1e20) {
            double shifted_ub = v.upper_bound - std_lp.var_shift[j];
            if (shifted_ub >= -1e-9) {
                int ub_row = m_total++;
                std_lp.b.push_back(std::max(0.0, shifted_ub));
                triplets.emplace_back(ub_row, model_var_to_std_idx[j], 1.0);

                int slack_idx = static_cast<int>(std_lp.c.size());
                std_lp.c.push_back(0.0);
                std_lp.lb.push_back(0.0);
                std_lp.ub.push_back(1e30);
                std_lp.orig_var_map.push_back(-1);
                triplets.emplace_back(ub_row, slack_idx, 1.0);
                std_lp.slack_rows.push_back(slack_idx);
            }
        }
    }

    // Flip rows where RHS < 0 so that b_i >= 0
    for (int i = 0; i < m_total; ++i) {
        if (std_lp.b[i] < -1e-12) {
            std_lp.b[i] = -std_lp.b[i];
            for (auto& t : triplets) {
                if (t.row == i) {
                    t.val = -t.val;
                }
            }
        }
    }

    int total_cols = static_cast<int>(std_lp.c.size());
    std_lp.A = SparseMatrix::from_triplets(m_total, total_cols, triplets);
    std_lp.A.build_csc();

    return std_lp;
}

SimplexResult SimplexSolver::solve_standard_lp(const StandardLP& std_lp,
                                              const std::vector<int>& initial_basis) {
    auto start_time = std::chrono::high_resolution_clock::now();
    SimplexResult res;

    int m = std_lp.A.num_rows;
    int n_orig = std_lp.A.num_cols;

    if (m == 0) {
        // No constraints: check if variables have non-zero obj and are unbounded
        res.status = SolverStatus::OPTIMAL;
        res.objective_value = (std_lp.orig_sense == ObjectiveSense::MAXIMIZE) ? -std_lp.obj_offset : std_lp.obj_offset;
        res.primal_solution.assign(std_lp.var_shift.size(), 0.0);
        for (size_t j = 0; j < std_lp.var_shift.size(); ++j) {
            res.primal_solution[j] = std_lp.var_shift[j];
        }
        return res;
    }

    // Identify Phase I initial basis and add artificial variables where needed
    std::vector<int> basis(m, -1);
    std::vector<int> artificial_cols;
    std::vector<MatrixTriplet> all_triplets;

    // Copy existing triplets from std_lp.A
    for (int r = 0; r < m; ++r) {
        int start = std_lp.A.row_ptr[r];
        int end = std_lp.A.row_ptr[r + 1];
        for (int k = start; k < end; ++k) {
            all_triplets.emplace_back(r, std_lp.A.col_indices[k], std_lp.A.values[k]);
        }
    }

    int next_col = n_orig;
    std::vector<double> phase1_c(n_orig, 0.0);
    std::vector<double> phase1_lb = std_lp.lb;
    std::vector<double> phase1_ub = std_lp.ub;

    for (int i = 0; i < m; ++i) {
        // Look for a standard unit column in row i
        bool found_unit = false;
        for (int j = 0; j < n_orig; ++j) {
            if (std_lp.A.get_element(i, j) == 1.0) {
                bool only_in_this_row = true;
                for (int r = 0; r < m; ++r) {
                    if (r != i && std::abs(std_lp.A.get_element(r, j)) > 1e-15) {
                        only_in_this_row = false;
                        break;
                    }
                }
                if (only_in_this_row && std_lp.lb[j] <= 0.0 && std_lp.b[i] >= 0.0) {
                    basis[i] = j;
                    found_unit = true;
                    break;
                }
            }
        }

        if (!found_unit) {
            // Add artificial variable
            int art_col = next_col++;
            basis[i] = art_col;
            artificial_cols.push_back(art_col);
            all_triplets.emplace_back(i, art_col, 1.0);
            phase1_c.push_back(1.0); // Cost = 1.0 in Phase I
            phase1_lb.push_back(0.0);
            phase1_ub.push_back(1e30);
        }
    }

    SparseMatrix full_A = SparseMatrix::from_triplets(m, next_col, all_triplets);
    full_A.build_csc();

    BasisMatrix basis_mat(m, next_col);
    basis_mat.basis_indices = basis;
    for (int j = 0; j < next_col; ++j) {
        basis_mat.var_status[j] = 1; // Nonbasic
    }
    for (int b_col : basis) {
        basis_mat.var_status[b_col] = 0; // Basic
    }

    if (!basis_mat.refactorize(full_A, tolerances.pivot_tolerance)) {
        res.status = SolverStatus::NUMERICAL_ERROR;
        res.diagnostics.add_warning("Initial basis factorization failed.");
        return res;
    }

    // ==========================================
    // PHASE I SIMPLEX
    // ==========================================
    std::vector<double> x_val(next_col, 0.0);
    // Initial basic values: x_B = b
    std::vector<double> x_B(m);
    for (int i = 0; i < m; ++i) {
        x_B[i] = std_lp.b[i];
        x_val[basis[i]] = x_B[i];
    }

    int iter = 0;
    pricing.initialize_weights(next_col);

    if (!artificial_cols.empty()) {
        while (iter < tolerances.max_iterations) {
            iter++;
            res.diagnostics.num_iterations++;

            // Check time limit
            auto now = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration<double>(now - start_time).count() > tolerances.time_limit_sec) {
                res.status = SolverStatus::TIME_LIMIT;
                return res;
            }

            // 1. Compute Phase I simplex multipliers: B^T pi = c_B
            std::vector<double> c_B(m, 0.0);
            for (int i = 0; i < m; ++i) {
                c_B[i] = phase1_c[basis_mat.basis_indices[i]];
            }
            std::vector<double> pi(m, 0.0);
            if (!basis_mat.btran(c_B, pi)) {
                basis_mat.refactorize(full_A, tolerances.pivot_tolerance);
                basis_mat.btran(c_B, pi);
            }

            // 2. Select entering variable
            std::vector<double> red_costs;
            int entering = pricing.select_entering_variable(full_A, phase1_c, pi, basis_mat,
                                                            phase1_lb, phase1_ub, x_val,
                                                            tolerances.optimality_tolerance, red_costs);

            if (entering == -1) {
                // Phase I Optimal
                break;
            }

            // 3. FTRAN: B * d = A_{.entering}
            std::vector<double> a_entering;
            full_A.get_column_dense(entering, a_entering);
            std::vector<double> d(m, 0.0);
            if (!basis_mat.ftran(a_entering, d)) {
                basis_mat.refactorize(full_A, tolerances.pivot_tolerance);
                basis_mat.ftran(a_entering, d);
            }

            // 4. Ratio test
            RatioTestResult ratio_res = RatioTest::harris_ratio_test(x_B, d, basis_mat.basis_indices,
                                                                    phase1_lb, phase1_ub, entering,
                                                                    tolerances.primal_feasibility_tolerance,
                                                                    tolerances.pivot_tolerance);

            if (ratio_res.is_unbounded) {
                // Phase I cannot be unbounded
                break;
            }

            if (ratio_res.is_bound_flip) {
                double theta = ratio_res.step_length;
                for (int i = 0; i < m; ++i) {
                    x_B[i] -= theta * d[i];
                    x_val[basis_mat.basis_indices[i]] = x_B[i];
                }
                x_val[entering] = phase1_ub[entering];
                continue;
            }

            int leaving_row = ratio_res.leaving_row;
            if (leaving_row < 0) break;

            if (std::abs(ratio_res.step_length) < 1e-12) {
                res.diagnostics.num_degenerate_pivots++;
            }
            res.diagnostics.min_pivot_size = std::min(res.diagnostics.min_pivot_size, std::abs(ratio_res.pivot_element));

            // Update basic solution
            double theta = ratio_res.step_length;
            for (int i = 0; i < m; ++i) {
                x_B[i] -= theta * d[i];
                x_val[basis_mat.basis_indices[i]] = x_B[i];
            }
            x_val[entering] = theta;

            // Pivot update
            pricing.update_weights(entering, leaving_row, d);
            basis_mat.update_basis(leaving_row, entering, full_A);
            x_B[leaving_row] = theta;
            x_val[basis_mat.basis_indices[leaving_row]] = theta;
        }

        // Check Phase I objective (sum of artificials)
        double phase1_obj = 0.0;
        for (int art_col : artificial_cols) {
            phase1_obj += x_val[art_col];
        }

        if (phase1_obj > tolerances.primal_feasibility_tolerance) {
            res.status = SolverStatus::INFEASIBLE;
            res.objective_value = 1e30;
            res.solve_time_sec = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start_time).count();
            return res;
        }

        // Lock all artificial variables to 0 for Phase II
        for (int art_col : artificial_cols) {
            phase1_ub[art_col] = 0.0;
            phase1_lb[art_col] = 0.0;
            x_val[art_col] = 0.0;
        }

        // Pivot out any artificial variables still in the basis
        for (int i = 0; i < m; ++i) {
            int b_var = basis_mat.basis_indices[i];
            if (b_var >= n_orig) {
                for (int j = 0; j < n_orig; ++j) {
                    if (!basis_mat.is_basic(j)) {
                        std::vector<double> a_j;
                        full_A.get_column_dense(j, a_j);
                        std::vector<double> d_j(m, 0.0);
                        if (basis_mat.ftran(a_j, d_j) && std::abs(d_j[i]) > tolerances.pivot_tolerance) {
                            basis_mat.update_basis(i, j, full_A);
                            break;
                        }
                    }
                }
            }
        }

        // Recompute exact basic solution x_B = B^{-1} b
        basis_mat.refactorize(std_lp.A, tolerances.pivot_tolerance);
        basis_mat.ftran(std_lp.b, x_B);
        x_val.assign(n_orig, 0.0);
        for (int i = 0; i < m; ++i) {
            if (basis_mat.basis_indices[i] >= 0 && basis_mat.basis_indices[i] < n_orig) {
                x_val[basis_mat.basis_indices[i]] = x_B[i];
            }
        }
    }

    // ==========================================
    // PHASE II SIMPLEX (Pure Model & Slack Space)
    // ==========================================
    std::vector<double> phase2_c = std_lp.c;
    phase2_c.resize(n_orig);
    std::vector<double> phase2_lb = std_lp.lb;
    phase2_lb.resize(n_orig);
    std::vector<double> phase2_ub = std_lp.ub;
    phase2_ub.resize(n_orig);

    pricing.initialize_weights(n_orig);

    int phase2_iters = 0;
    while (iter < tolerances.max_iterations) {
        iter++;
        phase2_iters++;
        res.diagnostics.num_iterations++;

        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration<double>(now - start_time).count() > tolerances.time_limit_sec) {
            res.status = SolverStatus::TIME_LIMIT;
            return res;
        }

        // 1. Simplex multipliers: B^T pi = c_B
        std::vector<double> c_B(m, 0.0);
        for (int i = 0; i < m; ++i) {
            int b_var = basis_mat.basis_indices[i];
            c_B[i] = (b_var >= 0 && b_var < static_cast<int>(phase2_c.size())) ? phase2_c[b_var] : 0.0;
        }
        std::vector<double> pi(m, 0.0);
        if (!basis_mat.btran(c_B, pi)) {
            basis_mat.refactorize(std_lp.A, tolerances.pivot_tolerance);
            basis_mat.btran(c_B, pi);
        }

        // 2. Pricing
        std::vector<double> red_costs;
        int entering = pricing.select_entering_variable(std_lp.A, phase2_c, pi, basis_mat,
                                                        phase2_lb, phase2_ub, x_val,
                                                        tolerances.optimality_tolerance, red_costs);

        if (entering == -1) {
            // Optimal!
            res.status = SolverStatus::OPTIMAL;
            res.dual_solution = pi;
            res.reduced_costs = red_costs;
            break;
        }

        // 3. FTRAN: B * d = A_{.entering}
        std::vector<double> a_entering;
        std_lp.A.get_column_dense(entering, a_entering);
        std::vector<double> d(m, 0.0);
        if (!basis_mat.ftran(a_entering, d)) {
            basis_mat.refactorize(std_lp.A, tolerances.pivot_tolerance);
            basis_mat.ftran(a_entering, d);
        }

        // 4. Ratio test
        RatioTestResult ratio_res = RatioTest::harris_ratio_test(x_B, d, basis_mat.basis_indices,
                                                                phase2_lb, phase2_ub, entering,
                                                                tolerances.primal_feasibility_tolerance,
                                                                tolerances.pivot_tolerance);

        if (ratio_res.is_unbounded) {
            res.status = SolverStatus::UNBOUNDED;
            res.objective_value = (std_lp.orig_sense == ObjectiveSense::MAXIMIZE) ? 1e30 : -1e30;
            return res;
        }

        int leaving_row = ratio_res.leaving_row;
        if (leaving_row < 0) {
            res.status = SolverStatus::OPTIMAL;
            break;
        }

        if (std::abs(ratio_res.step_length) < 1e-12) {
            res.diagnostics.num_degenerate_pivots++;
        }
        res.diagnostics.min_pivot_size = std::min(res.diagnostics.min_pivot_size, std::abs(ratio_res.pivot_element));

        // Update solution
        double theta = ratio_res.step_length;
        for (int i = 0; i < m; ++i) {
            x_B[i] -= theta * d[i];
            x_val[basis_mat.basis_indices[i]] = x_B[i];
        }
        x_val[entering] = theta;

        pricing.update_weights(entering, leaving_row, d);
        basis_mat.update_basis(leaving_row, entering, std_lp.A);
        x_B[leaving_row] = theta;
        x_val[basis_mat.basis_indices[leaving_row]] = theta;
    }

    if (iter >= tolerances.max_iterations && res.status != SolverStatus::OPTIMAL) {
        res.status = SolverStatus::ITERATION_LIMIT;
        res.diagnostics.add_warning("Solver reached maximum iteration limit.");
    }

    // Extract Primal Solution
    int num_orig_vars = static_cast<int>(std_lp.var_shift.size());
    res.primal_solution.assign(num_orig_vars, 0.0);
    for (int j = 0; j < n_orig; ++j) {
        int orig_idx = std_lp.orig_var_map[j];
        if (orig_idx >= 0 && orig_idx < num_orig_vars) {
            res.primal_solution[orig_idx] = x_val[j] + std_lp.var_shift[orig_idx];
        }
    }

    // Calculate Final Objective
    double raw_obj = std_lp.obj_offset;
    for (int j = 0; j < n_orig; ++j) {
        raw_obj += std_lp.c[j] * x_val[j];
    }

    if (std_lp.orig_sense == ObjectiveSense::MAXIMIZE) {
        res.objective_value = -raw_obj;
    } else {
        res.objective_value = raw_obj;
    }

    res.basic_variables = basis_mat.basis_indices;
    res.diagnostics.max_condition_number = basis_mat.current_rcond > 1e-15 ? (1.0 / basis_mat.current_rcond) : 1e12;
    if (res.diagnostics.max_condition_number > tolerances.condition_threshold_warning) {
        res.diagnostics.add_warning("Model appears poorly conditioned (kappa estimate > 1e9).");
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.solve_time_sec = std::chrono::duration<double>(end_time - start_time).count();

    return res;
}

SimplexResult SimplexSolver::solve(const Model& model) {
    StandardLP std_lp = transform_to_standard_form(model);
    return solve_standard_lp(std_lp);
}

} // namespace hunters
