#include "core/parsers/capacitor_parser.hpp"
#include "core/parsers/parser_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::unordered_map<std::string, Capacitor> CapacitorParser::parse(const std::string& filename) {
    std::unordered_map<std::string, Capacitor> capacitors;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Capacitor file: " + filename);
    }

    std::string line, word;
    std::getline(file, line); // skip header
    int line_num = 1;

    while (std::getline(file, line)) {
        ++line_num;
        if (line.empty() || line.find_first_not_of("\r\n\t ") == std::string::npos) {
            continue;
        }

        std::stringstream ss(line);
        Capacitor cap;

        try {
            std::getline(ss, cap.node, ','); parser_utils::trim_token(cap.node);
            if (cap.node.empty()) throw std::runtime_error("missing node name");

            for (int phase = 0; phase < 3; phase++) {
                std::getline(ss, word, ','); parser_utils::trim_token(word);
                if (word.empty()) throw std::runtime_error("missing kVAr for phase " + std::to_string(phase));
                cap.kvar[phase] = std::stod(word);
            }

            capacitors[cap.node] = cap;
        } catch (const std::exception& e) {
            std::cerr << "capacitor_parser: skipping malformed line " << line_num
                      << " [" << line << "]: " << e.what() << '\n';
        }
    }

    return capacitors;
}