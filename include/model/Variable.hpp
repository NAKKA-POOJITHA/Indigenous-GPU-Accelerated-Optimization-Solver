#ifndef HUNTERS_MODEL_VARIABLE_HPP
#define HUNTERS_MODEL_VARIABLE_HPP

#include <string>
#include <limits>
#include <cmath>

namespace hunters {

enum class VarType {
    CONTINUOUS,
    INTEGER,
    BINARY
};

enum class ObjectiveSense {
    MINIMIZE,
    MAXIMIZE
};

struct Variable {
    int id;
    std::string name;
    double lower_bound;
    double upper_bound;
    double obj_coefficient;
    VarType type;

    static constexpr double INF = 1e30;

    Variable(int id_ = -1,
             const std::string& name_ = "",
             double lb = 0.0,
             double ub = INF,
             double obj = 0.0,
             VarType t = VarType::CONTINUOUS)
        : id(id_), name(name_), lower_bound(lb), upper_bound(ub), obj_coefficient(obj), type(t) {
        if (type == VarType::BINARY) {
            lower_bound = (lb < 0.0) ? 0.0 : lb;
            upper_bound = (ub > 1.0) ? 1.0 : ub;
        }
    }

    bool is_integer() const {
        return type == VarType::INTEGER || type == VarType::BINARY;
    }

    bool is_binary() const {
        return type == VarType::BINARY;
    }

    bool is_fixed() const {
        return std::abs(upper_bound - lower_bound) < 1e-12;
    }
};

} // namespace hunters

#endif // HUNTERS_MODEL_VARIABLE_HPP
