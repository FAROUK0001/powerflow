#pragma once
#include <string>

struct Capacitor {
    std::string node;
    // Arrays for Phase A, Phase B, Phase C
    double kvar[3];
};