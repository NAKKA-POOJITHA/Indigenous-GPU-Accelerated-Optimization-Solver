#ifndef HUNTERS_SPARSE_VECTOR_OPS_HPP
#define HUNTERS_SPARSE_VECTOR_OPS_HPP

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace hunters {

class VectorOps {
public:
    static double dot(const std::vector<double>& a, const std::vector<double>& b) {
        size_t n = std::min(a.size(), b.size());
        double sum = 0.0;
        for (size_t i = 0; i < n; ++i) {
            sum += a[i] * b[i];
        }
        return sum;
    }

    static double norm_inf(const std::vector<double>& v) {
        double max_val = 0.0;
        for (double x : v) {
            double ax = std::abs(x);
            if (ax > max_val) max_val = ax;
        }
        return max_val;
    }

    static double norm_1(const std::vector<double>& v) {
        double sum = 0.0;
        for (double x : v) {
            sum += std::abs(x);
        }
        return sum;
    }

    static double norm_2(const std::vector<double>& v) {
        double sum_sq = 0.0;
        for (double x : v) {
            sum_sq += x * x;
        }
        return std::sqrt(sum_sq);
    }

    // y = alpha * x + y
    static void axpy(double alpha, const std::vector<double>& x, std::vector<double>& y) {
        size_t n = std::min(x.size(), y.size());
        for (size_t i = 0; i < n; ++i) {
            y[i] += alpha * x[i];
        }
    }

    // Compute residual: r = b - Ax
    static void compute_residual(const std::vector<double>& b,
                                 const std::vector<double>& Ax,
                                 std::vector<double>& residual) {
        residual.resize(b.size());
        for (size_t i = 0; i < b.size(); ++i) {
            residual[i] = b[i] - Ax[i];
        }
    }

    static void scale(std::vector<double>& v, double s) {
        for (double& x : v) {
            x *= s;
        }
    }
};

} // namespace hunters

#endif // HUNTERS_SPARSE_VECTOR_OPS_HPP
