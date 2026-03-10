#pragma once
#include <string>
#include <vector>
#include "core/models/distributed_load.hpp"

class DistributedLoadParser {
public:
    static std::vector<DistributedLoad> parse(const std::string& filename);
};