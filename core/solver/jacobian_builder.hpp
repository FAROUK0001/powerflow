#pragma once

#include <vector>
#include <complex>
#include "core/linalg/matrix_sparse_csr.hpp"
#include "core/linalg/matrix_dense.hpp"

// We define a 2D vector to represent our massive Real-Number Matrix
using RealMatrix = std::vector<std::vector<double>>;
using ComplexMatrix3x3 = MatrixDense<std::complex<double>>;
using ComplexVector3 = std::vector<std::complex<double>>;

class JacobianBuilder {
public:
    static RealMatrix build(
        const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
        const std::vector<ComplexVector3>& V,
        const std::vector<ComplexVector3>& S_calc,
        int num_nodes
    );
};