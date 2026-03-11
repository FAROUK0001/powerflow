#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <complex>
#include "core/models/branch.hpp"
#include "core/models/phase_config.hpp"
#include "core/models/transformer.hpp"
#include "core/linalg/matrix_dense.hpp"
#include "core/models/capacitor.hpp"

using ComplexMatrix3x3 = MatrixDense<std::complex<double>>;

class YBusBuilder {
public:
    static std::unordered_map<int, std::unordered_map<int, ComplexMatrix3x3>>
    build_ybus_map(
        const std::vector<Branch>& branches,
        const std::unordered_map<std::string, PhaseConfig>& configs,
        const std::unordered_map<std::string, Capacitor>& capacitors,
        const std::unordered_map<std::string, Transformer>& transformers,
        const std::unordered_map<std::string, int>& node_to_index
    );
};