#ifndef HUNTERS_MILP_BRANCHING_RULES_HPP
#define HUNTERS_MILP_BRANCHING_RULES_HPP

#include "model/Model.hpp"
#include "milp/Node.hpp"
#include <vector>

namespace hunters {

class BranchingRules {
public:
    static int select_branching_variable(const Model& model,
                                         const std::vector<double>& solution,
                                         double integrality_tol,
                                         BranchingStrategy strategy = BranchingStrategy::MOST_FRACTIONAL);
};

} // namespace hunters

#endif // HUNTERS_MILP_BRANCHING_RULES_HPP
