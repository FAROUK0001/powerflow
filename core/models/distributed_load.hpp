#pragma once
#include <string>

struct DistributedLoad {
    std::string node_a;
    std::string node_b;
    std::string model; // e.g., "Y-PQ", "D-I"

    // Arrays for Phase 1, Phase 2, Phase 3
    double kw[3];
    double kvar[3];
};