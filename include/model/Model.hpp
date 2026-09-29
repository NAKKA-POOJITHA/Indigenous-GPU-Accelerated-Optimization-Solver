#ifndef HUNTERS_MODEL_MODEL_HPP
#define HUNTERS_MODEL_MODEL_HPP

#include "Variable.hpp"
#include "Constraint.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace hunters {

class Model {
public:
    std::string name;
    ObjectiveSense sense;
    double obj_offset;

    std::vector<Variable> variables;
    std::vector<Constraint> constraints;

    std::unordered_map<std::string, int> var_name_to_idx;
    std::unordered_map<std::string, int> constr_name_to_idx;

    Model(const std::string& model_name = "HuntersModel")
        : name(model_name), sense(ObjectiveSense::MINIMIZE), obj_offset(0.0) {}

    int add_variable(const std::string& var_name,
                     double lb = 0.0,
                     double ub = Variable::INF,
                     double obj = 0.0,
                     VarType type = VarType::CONTINUOUS) {
        int idx = static_cast<int>(variables.size());
        std::string final_name = var_name.empty() ? ("x_" + std::to_string(idx)) : var_name;
        variables.emplace_back(idx, final_name, lb, ub, obj, type);
        var_name_to_idx[final_name] = idx;
        return idx;
    }

    int add_constraint(const std::string& constr_name,
                       const std::vector<int>& var_indices,
                       const std::vector<double>& coefs,
                       ConstraintSense c_sense,
                       double rhs) {
        int idx = static_cast<int>(constraints.size());
        std::string final_name = constr_name.empty() ? ("c_" + std::to_string(idx)) : constr_name;
        Constraint c(idx, final_name, c_sense, rhs);
        for (size_t i = 0; i < var_indices.size(); ++i) {
            c.add_term(var_indices[i], coefs[i]);
        }
        constraints.push_back(std::move(c));
        constr_name_to_idx[final_name] = idx;
        return idx;
    }

    void set_objective_sense(ObjectiveSense s) {
        sense = s;
    }

    void maximize() {
        sense = ObjectiveSense::MAXIMIZE;
    }

    void minimize() {
        sense = ObjectiveSense::MINIMIZE;
    }

    size_t num_variables() const { return variables.size(); }
    size_t num_constraints() const { return constraints.size(); }

    size_t num_integer_variables() const {
        size_t count = 0;
        for (const auto& v : variables) {
            if (v.is_integer()) ++count;
        }
        return count;
    }

    size_t num_nonzeros() const {
        size_t count = 0;
        for (const auto& c : constraints) {
            count += c.num_nonzeros();
        }
        return count;
    }

    bool is_milp() const {
        for (const auto& v : variables) {
            if (v.is_integer()) return true;
        }
        return false;
    }

    void print_summary() const;
};

} // namespace hunters

#endif // HUNTERS_MODEL_MODEL_HPP
