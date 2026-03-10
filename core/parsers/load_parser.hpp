#pragma once
#include <string>
#include <unordered_map>
#include "core/models/load.hpp"

class LoadParser {
public:
    // We return a dictionary that maps the Node Name (e.g. "860") to its Load data
    static std::unordered_map<std::string, Load> parse(const std::string& filename);
};