#pragma once
#include <string>
#include <unordered_map>
#include "core/models/phase_config.hpp"

class PhaseConfigParser {
public:
    static std::unordered_map<std::string, PhaseConfig> parse(const std::string& filepath);
};