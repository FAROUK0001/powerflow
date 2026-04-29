#pragma once

#include <vector>
#include <array>
#include <complex>
#include "core/linalg/matrix_sparse_csr.hpp"
#include "core/linalg/matrix_dense.hpp"

// We define a 2D vector to represent our massive Real-Number Matrix
using RealMatrix = std::vector<std::vector<double>>;
using ComplexMatrix3x3 = MatrixDense<std::complex<double>>;
using ComplexVector3 = std::vector<std::complex<double>>;

// Maps (node, phase) -> base offset in the state vector.
// Each active phase occupies two consecutive slots: [base] = theta, [base+1] = |V|.
// idx[node][phase] == -1 means the phase is absent at that node.
struct StateIndex {
    std::vector<std::array<int, 3>> idx; // [node][phase] -> base offset, or -1
    int dimension{0};                    // total number of state variables
};

class JacobianBuilder {
public:
    static RealMatrix build(
        const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
        const std::vector<ComplexVector3>& V,
        const std::vector<ComplexVector3>& S_calc,
        const StateIndex& state_index
    );
};