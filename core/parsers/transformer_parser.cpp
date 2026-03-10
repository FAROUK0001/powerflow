#include "core/parsers/transformer_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::unordered_map<std::string, Transformer> TransformerParser::parse(const std::string& filename) {
    std::unordered_map<std::string, Transformer> transformers;
    std::ifstream file(filename);
    std::string line, word;

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Transformer file: " + filename);
    }

    std::getline(file, line); // Throwaway the title!

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        Transformer tx;

        std::getline(ss, tx.name, ',');
        std::getline(ss, word, ','); tx.kva = std::stod(word);
        std::getline(ss, word, ','); tx.kv_high = std::stod(word);
        std::getline(ss, word, ','); tx.kv_low = std::stod(word);
        std::getline(ss, word, ','); tx.r_percent = std::stod(word);
        std::getline(ss, word, ','); tx.x_percent = std::stod(word);

        transformers[tx.name] = tx;
    }

    return transformers;
}