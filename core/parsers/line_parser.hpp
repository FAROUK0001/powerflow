#pragma once
#include <string>
#include <vector>
#include "core/models/branch.hpp"

class LineParser {
public:
    static std::vector<Branch> parse(const std::string& filepath);
};