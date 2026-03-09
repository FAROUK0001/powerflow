#pragma once
#include <string>
#include <complex>
#include "core/linalg/matrix_dense.hpp"

struct Branch {
    // 1. The Nodes (Strings because IEEE nodes are sometimes "800", "808", etc.)
    std::string node_a;
    std::string node_b;

    // 2. Physical Properties from line_data.csv
    double length_ft;
    std::string config_id; // e.g., "300", "301"

    // 3. The 3x3 Phase Matrices (Filled later by looking up config_id in phase_configs.csv)
    // Z_abc = R_abc + jX_abc
    MatrixDense<std::complex<double>> Z_abc{3, 3};

    // Y_abc = 0 + jB_abc (Line Charging)
    MatrixDense<std::complex<double>> Y_abc{3, 3};
};