#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <complex>
#include "core/models/branch.hpp"
#include "core/models/phase_config.hpp"
#include "core/linalg/matrix_dense.hpp"

// We use a custom type alias so we don't have to type this huge thing every time
using ComplexMatrix3x3 = MatrixDense<std::complex<double>>;

class YBusBuilder {
public:
    // This builds a temporary "Map-based" Y-Bus that is super easy to read and print!
    static std::unordered_map<int, std::unordered_map<int, ComplexMatrix3x3>>
    build_ybus_map(
        const std::vector<Branch>& branches,
        const std::unordered_map<std::string, PhaseConfig>& configs,
        const std::unordered_map<std::string, int>& node_to_index
    );
};