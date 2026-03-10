#include "core/parsers/regulator_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<Regulator> RegulatorParser::parse(const std::string& filename) {
    std::vector<Regulator> regulators;
    std::ifstream file(filename);
    std::string line, word;

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Regulator file: " + filename);
    }

    // 1. THROWAWAY READ (Skips the Header!)
    std::getline(file, line);

    // 2. MAIN LOOP
    while (std::getline(file, line)) {
        if (line.empty() || line.find_first_not_of("\r\n\t ") == std::string::npos) {
            continue;
        }

        std::stringstream ss(line);
        Regulator reg;

        // Columns 1, 2, & 3: Strings
        std::getline(ss, reg.id, ',');
        std::getline(ss, reg.from_node, ',');
        std::getline(ss, reg.to_node, ',');

        // Column 4: Integer (Phase)
        std::getline(ss, word, ',');
        reg.phase = std::stoi(word); // String to Integer!

        // Columns 5 to 10: Doubles
        std::getline(ss, word, ','); reg.v_hold = std::stod(word);
        std::getline(ss, word, ','); reg.r_volt = std::stod(word);
        std::getline(ss, word, ','); reg.x_volt = std::stod(word);
        std::getline(ss, word, ','); reg.pt_ratio = std::stod(word);
        std::getline(ss, word, ','); reg.ct_rate = std::stod(word);
        std::getline(ss, word, ','); reg.bandwidth = std::stod(word);

        regulators.push_back(reg); // Add it to our list!
    }

    return regulators;
}