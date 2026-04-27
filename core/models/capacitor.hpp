#pragma once
#include <array>
#include <string>

struct Capacitor {
    std::string node;
    // Arrays for Phase A, Phase B, Phase C
    std::array<double, 3> kvar{};
};