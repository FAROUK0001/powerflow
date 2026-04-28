#pragma once
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

// Convergence utilities for the Newton-Raphson power-flow solver.
namespace convergence {

// Compute the infinity-norm (max absolute value) of the complex power mismatch
// for all non-slack nodes (i.e. nodes starting at index start_node).
inline double mismatch_norm(
    const std::vector<std::vector<std::complex<double>>>& mismatch,
    int start_node = 1)
{
    double max_val = 0.0;
    for (int i = start_node; i < static_cast<int>(mismatch.size()); ++i) {
        for (const auto& s : mismatch[i]) {
            max_val = std::max(max_val, std::abs(s));
        }
    }
    return max_val;
}

// Return true when the mismatch norm is below the given tolerance.
inline bool has_converged(double norm, double tolerance) {
    return norm < tolerance;
}

} // namespace convergence
