#ifndef HUNTERS_LP_RATIO_TEST_HPP
#define HUNTERS_LP_RATIO_TEST_HPP

#include <vector>

namespace hunters {

struct RatioTestResult {
    int leaving_row;       // Row index in basis that leaves (0..m-1), or -1 if bound flip
    double step_length;    // Step length theta
    bool is_unbounded;     // True if ray is unbounded
    bool is_bound_flip;    // True if entering variable hits its own upper bound
    double pivot_element;  // Value of d_p
};

class RatioTest {
public:
    // Performs Harris Two-Pass ratio test for entering variable increasing from lower bound
    static RatioTestResult harris_ratio_test(const std::vector<double>& x_B,
                                            const std::vector<double>& d,
                                            const std::vector<int>& basis_indices,
                                            const std::vector<double>& lower_bounds,
                                            const std::vector<double>& upper_bounds,
                                            int entering_col,
                                            double feasibility_tol,
                                            double pivot_tol);
};

} // namespace hunters

#endif // HUNTERS_LP_RATIO_TEST_HPP
