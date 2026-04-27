#include "core/parsers/line_parser.hpp"
#include "core/parsers/parser_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::vector<Branch> LineParser::parse(const std::string& filepath) {
    std::vector<Branch> branches;
    std::ifstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);

    std::string line;
    std::getline(file, line); // skip header
    int line_num = 1;

    while (std::getline(file, line)) {
        ++line_num;
        if (line.empty() || line.find_first_not_of("\r\n\t ") == std::string::npos) continue;

        std::stringstream ss(line);
        std::string piece;
        Branch b;

        try {
            std::getline(ss, b.node_a, ',');    parser_utils::trim_token(b.node_a);
            std::getline(ss, b.node_b, ',');    parser_utils::trim_token(b.node_b);
            std::getline(ss, piece, ',');        parser_utils::trim_token(piece);
            std::getline(ss, b.config_id, ','); parser_utils::trim_token(b.config_id);

            if (b.node_a.empty() || b.node_b.empty())
                throw std::runtime_error("missing node name");
            if (piece.empty())
                throw std::runtime_error("missing length");
            if (b.config_id.empty())
                throw std::runtime_error("missing config_id");

            b.length_ft = std::stod(piece);
            branches.push_back(b);
        } catch (const std::exception& e) {
            std::cerr << "line_parser: skipping malformed line " << line_num
                      << " [" << line << "]: " << e.what() << '\n';
        }
    }
    return branches;
}