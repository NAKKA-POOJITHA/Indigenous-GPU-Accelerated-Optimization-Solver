#ifndef HUNTERS_LP_PRICING_HPP
#define HUNTERS_LP_PRICING_HPP

#include "sparse/SparseMatrix.hpp"
#include "lp/BasisMatrix.hpp"
#include <vector>

namespace hunters {

enum class PricingStrategy {
    DANTZIG,
    DEVEX_STEEPEST_EDGE,
    BLANDS_RULE
};

class Pricing {
public:
    PricingStrategy strategy;
    std::vector<double> devex_weights; // Approximate steepest-edge weights

    Pricing(PricingStrategy strat = PricingStrategy::DEVEX_STEEPEST_EDGE)
        : strategy(strat) {}

    void initialize_weights(int num_cols);

    // Compute reduced costs and select entering variable
    // Returns column index of entering variable, or -1 if optimal
    int select_entering_variable(const SparseMatrix& A,
                                 const std::vector<double>& c,
                                 const std::vector<double>& pi,
                                 const BasisMatrix& basis,
                                 const std::vector<double>& lower_bounds,
                                 const std::vector<double>& upper_bounds,
                                 const std::vector<double>& x_vals,
                                 double optimality_tol,
                                 std::vector<double>& reduced_costs);

    void update_weights(int entering_col, int leaving_row, const std::vector<double>& d);
};

} // namespace hunters

#endif // HUNTERS_LP_PRICING_HPP
