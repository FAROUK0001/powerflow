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

// Create a struct to hold all the math results!
struct SolverResult {
    std::vector<ComplexVector3> V;
    std::vector<ComplexVector3> S_spec;
    std::vector<ComplexVector3> S_calc;
    std::vector<ComplexVector3> mismatch;
    RealMatrix J;
};

class NewtonRaphson {
public:
    static SolverResult solve(
        const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
        const std::unordered_map<std::string, Load>& spot_loads,
        const std::vector<DistributedLoad>& dist_loads,
        const std::unordered_map<std::string, int>& node_to_index
    );
};