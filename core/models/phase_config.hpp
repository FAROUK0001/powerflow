#pragma once
#include <string>
#include <complex>
#include "core/linalg/matrix_dense.hpp"

struct PhaseConfig {
    std::string config_id; // e.g., "300"

    // The raw 3x3 matrices read directly from phase_configs.csv (in Ohms per mile or ft)
    MatrixDense<std::complex<double>> Z_matrix{3, 3};
    MatrixDense<std::complex<double>> B_matrix{3, 3};
};