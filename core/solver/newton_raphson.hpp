#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <complex>

#include "core/linalg/matrix_sparse_csr.hpp"
#include "core/linalg/matrix_dense.hpp"
#include "core/models/load.hpp"
#include "core/models/distributed_load.hpp"
#include "core/solver/jacobian_builder.hpp"


using ComplexMatrix3x3 = MatrixDense<std::complex<double>>;
using ComplexVector3 = std::vector<std::complex<double>>;

// Holds all results produced by the Newton-Raphson solver.
struct SolverResult {
    std::vector<ComplexVector3> V;          // Final bus voltages
    std::vector<ComplexVector3> S_spec;     // Specified (load) power injections
    std::vector<ComplexVector3> S_calc;     // Calculated power from final voltages
    std::vector<ComplexVector3> mismatch;   // S_spec - S_calc (final)
    RealMatrix J;                           // Jacobian at final iteration
    int  iterations{0};                     // Number of NR iterations performed
    double mismatch_norm{0.0};              // Infinity-norm of final mismatch (VA)
    bool converged{false};                  // True if tolerance was met
};

class NewtonRaphson {
public:
    // Default solver parameters
    static constexpr int    DEFAULT_MAX_ITER  = 50;
    static constexpr double DEFAULT_TOLERANCE = 1.0; // 1 VA

    static SolverResult solve(
        const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
        const std::unordered_map<std::string, Load>& spot_loads,
        const std::vector<DistributedLoad>& dist_loads,
        const std::unordered_map<std::string, int>& node_to_index,
        int    max_iter  = DEFAULT_MAX_ITER,
        double tolerance = DEFAULT_TOLERANCE
    );
};