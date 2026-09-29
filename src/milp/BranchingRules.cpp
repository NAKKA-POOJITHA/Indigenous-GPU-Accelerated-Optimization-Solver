#include "milp/BranchingRules.hpp"
#include <cmath>
#include <algorithm>

namespace hunters {

int BranchingRules::select_branching_variable(const Model& model,
                                             const std::vector<double>& solution,
                                             double integrality_tol,
                                             BranchingStrategy strategy) {
    int best_var = -1;
    double max_fractionality = 0.0;

    for (size_t j = 0; j < model.variables.size(); ++j) {
        const auto& v = model.variables[j];
        if (!v.is_integer()) continue;

        double xj = solution[j];
        double rounded = std::round(xj);
        double dist = std::abs(xj - rounded);

        if (dist > integrality_tol) {
            double frac = xj - std::floor(xj);
            double fractionality = std::min(frac, 1.0 - frac); // Distance to nearest integer (max is 0.5)

            if (fractionality > max_fractionality) {
                max_fractionality = fractionality;
                best_var = static_cast<int>(j);
            }
        }
    }

    return best_var;
}

} // namespace hunters
