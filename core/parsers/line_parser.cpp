#include "core/parsers/line_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::vector<Branch> LineParser::parse(const std::string& filepath) {
    std::vector<Branch> branches;
    std::ifstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string piece;
        Branch b;

        try {
            std::getline(ss, b.node_a, ',');
            std::getline(ss, b.node_b, ',');
            std::getline(ss, piece, ','); b.length_ft = std::stod(piece);
            std::getline(ss, b.config_id, ',');
            if (!b.config_id.empty() && b.config_id.back() == '\r') b.config_id.pop_back();

            branches.push_back(b);
        } catch (...) {}
    }
    return branches;
}