#pragma once
#include <array>
#include <string>

struct Load {
    std::string node;
    std::string model; // e.g., "Y-PQ", "D-Z"

    // Arrays to hold the 3 phases [Phase 1, Phase 2, Phase 3]
    std::array<double, 3> kw{};
    std::array<double, 3> kvar{};
};