#pragma once
#include <string>
#include <vector>
#include "core/models/regulator.hpp"

class RegulatorParser {
public:
    static std::vector<Regulator> parse(const std::string& filename);
};