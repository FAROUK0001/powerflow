#pragma once
#include <string>
#include <unordered_map>
#include "core/models/capacitor.hpp"

class CapacitorParser {
public:
    static std::unordered_map<std::string, Capacitor> parse(const std::string& filename);
};