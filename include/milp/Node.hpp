#ifndef HUNTERS_MILP_NODE_HPP
#define HUNTERS_MILP_NODE_HPP

#include <vector>
#include <utility>

namespace hunters {

struct Node {
    int id;
    int parent_id;
    int depth;
    double lower_bound; // Dual bound from parent LP relaxation
    std::vector<std::pair<int, double>> custom_lower_bounds;
    std::vector<std::pair<int, double>> custom_upper_bounds;
    std::vector<int> warm_basis;

    Node(int id_ = 0, int parent_ = -1, int d = 0, double bound = -1e30)
        : id(id_), parent_id(parent_), depth(d), lower_bound(bound) {}
};

enum class NodeSelectionStrategy {
    BEST_BOUND,
    DEPTH_FIRST
};

enum class BranchingStrategy {
    MOST_FRACTIONAL,
    STRONG_BRANCHING
};

} // namespace hunters

#endif // HUNTERS_MILP_NODE_HPP
