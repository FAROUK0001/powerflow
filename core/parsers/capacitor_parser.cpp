#include "core/parsers/capacitor_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::unordered_map<std::string, Capacitor> CapacitorParser::parse(const std::string& filename) {
    std::unordered_map<std::string, Capacitor> capacitors;
    std::ifstream file(filename);
    std::string line, word;

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Capacitor file: " + filename);
    }

    // 1. THROWAWAY READ (Skips the Header: Node,PhA_kVAr,PhB_kVAr,PhC_kVAr)
    std::getline(file, line);

    // 2. MAIN LOOP
    while (std::getline(file, line)) {
        if (line.empty() || line.find_first_not_of("\r\n\t ") == std::string::npos) {
            continue;
        }

        std::stringstream ss(line);
        Capacitor cap;

        // Column 1: String
        std::getline(ss, cap.node, ',');

        // Columns 2, 3, 4: Doubles (kVAr for Phases A, B, C)
        std::getline(ss, word, ','); cap.kvar[0] = std::stod(word);
        std::getline(ss, word, ','); cap.kvar[1] = std::stod(word);
        std::getline(ss, word, ','); cap.kvar[2] = std::stod(word);

        // Add it to our dictionary!
        capacitors[cap.node] = cap;
    }

    return capacitors;
}