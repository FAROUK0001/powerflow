#pragma once
#include <string>
#include <unordered_map>
#include "core/models/transformer.hpp"

class TransformerParser {
public:
    static std::unordered_map<std::string, Transformer> parse(const std::string& filename);
};