#ifndef HUNTERS_PRESOLVE_PRESOLVER_HPP
#define HUNTERS_PRESOLVE_PRESOLVER_HPP

#include "model/Model.hpp"
#include <vector>
#include <string>
#include <iostream>

namespace hunters {

struct PresolveSummary {
    size_t orig_variables = 0;
    size_t orig_constraints = 0;
    size_t orig_nonzeros = 0;

    size_t removed_variables = 0;
    size_t removed_constraints = 0;
    size_t fixed_variables = 0;
    size_t tightened_bounds = 0;

    size_t reduced_variables = 0;
    size_t reduced_constraints = 0;
    size_t reduced_nonzeros = 0;

    double presolve_time_sec = 0.0;
    bool is_infeasible = false;
    bool is_unbounded = false;

    void print_summary() const {
        std::cout << "Original variables   : " << orig_variables << std::endl;
        std::cout << "Original constraints : " << orig_constraints << std::endl;
        std::cout << "Original nonzeros    : " << orig_nonzeros << std::endl;
        std::cout << std::endl;
        std::cout << "Presolve:" << std::endl;
        std::cout << "Removed variables    : " << removed_variables << std::endl;
        std::cout << "Removed constraints  : " << removed_constraints << std::endl;
        std::cout << "Fixed variables      : " << fixed_variables << std::endl;
        std::cout << "Tightened bounds     : " << tightened_bounds << std::endl;
        std::cout << std::endl;
        std::cout << "Reduced variables    : " << reduced_variables << std::endl;
        std::cout << "Reduced constraints  : " << reduced_constraints << std::endl;
        std::cout << "Reduced nonzeros     : " << reduced_nonzeros << std::endl;
        if (is_infeasible) {
            std::cout << "Status               : PROVEN INFEASIBLE IN PRESOLVE" << std::endl;
        }
    }
};

struct FixedVarRecord {
    int orig_var_idx;
    double fixed_val;
};

class Presolver {
public:
    PresolveSummary summary;
    std::vector<FixedVarRecord> fixed_history;
    std::vector<int> orig_to_reduced_var_map;
    std::vector<int> reduced_to_orig_var_map;

    // Presolve the model, producing a reduced Model
    Model presolve(const Model& orig_model);

    // Postsolve: Reconstruct original solution from reduced solution
    std::vector<double> postsolve(const Model& orig_model, const std::vector<double>& reduced_solution) const;
};

} // namespace hunters

#endif // HUNTERS_PRESOLVE_PRESOLVER_HPP
