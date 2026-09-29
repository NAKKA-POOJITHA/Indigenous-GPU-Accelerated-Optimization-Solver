#ifndef HUNTERS_MODEL_CONSTRAINT_HPP
#define HUNTERS_MODEL_CONSTRAINT_HPP

#include <string>
#include <vector>
#include <utility>
#include <cmath>

namespace hunters {

enum class ConstraintSense {
    LESS_EQUAL,    // <=
    GREATER_EQUAL, // >=
    EQUAL,         // ==
    RANGE          // lower_bound <= expr <= upper_bound
};

struct Constraint {
    int id;
    std::string name;
    std::vector<int> var_indices;
    std::vector<double> coefficients;
    ConstraintSense sense;
    double rhs;
    double lower_bound; // For RANGE constraints
    double upper_bound;

    static constexpr double INF = 1e30;

    Constraint(int id_ = -1,
               const std::string& name_ = "",
               ConstraintSense sense_ = ConstraintSense::LESS_EQUAL,
               double rhs_ = 0.0)
        : id(id_), name(name_), sense(sense_), rhs(rhs_), lower_bound(-INF), upper_bound(INF) {
        if (sense == ConstraintSense::LESS_EQUAL) {
            lower_bound = -INF;
            upper_bound = rhs;
        } else if (sense == ConstraintSense::GREATER_EQUAL) {
            lower_bound = rhs;
            upper_bound = INF;
        } else if (sense == ConstraintSense::EQUAL) {
            lower_bound = rhs;
            upper_bound = rhs;
        }
    }

    void add_term(int var_idx, double coef) {
        if (std::abs(coef) > 1e-15) {
            var_indices.push_back(var_idx);
            coefficients.push_back(coef);
        }
    }

    size_t num_nonzeros() const {
        return var_indices.size();
    }
};

} // namespace hunters

#endif // HUNTERS_MODEL_CONSTRAINT_HPP
