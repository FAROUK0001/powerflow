#include "core/parsers/load_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::unordered_map<std::string, Load> LoadParser::parse(const std::string& filename) {
    std::unordered_map<std::string, Load> loads;
    std::ifstream file(filename);
    std::string line, word;

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Load file: " + filename);
    }

    // 1. THE THROWAWAY READ (Skips the Header: Node,Load_Model,Ph1_kW...)
    std::getline(file, line);

    // 2. THE MAIN LOOP
    while (std::getline(file, line)) {
        // Skip empty lines at the bottom of the file
        if (line.empty() || line.find_first_not_of("\r\n\t ") == std::string::npos) {
            continue;
        }

        std::stringstream ss(line);
        Load l;

        // Column 1 & 2: Strings
        std::getline(ss, l.node, ',');
        std::getline(ss, l.model, ',');

        // Column 3 & 4: Phase 1 (kW and kVAr)
        std::getline(ss, word, ','); l.kw[0] = std::stod(word);
        std::getline(ss, word, ','); l.kvar[0] = std::stod(word);

        // Column 5 & 6: Phase 2 (kW and kVAr)
        std::getline(ss, word, ','); l.kw[1] = std::stod(word);
        std::getline(ss, word, ','); l.kvar[1] = std::stod(word);

        // Column 7 & 8: Phase 3 (kW and kVAr)
        std::getline(ss, word, ','); l.kw[2] = std::stod(word);
        std::getline(ss, word, ','); l.kvar[2] = std::stod(word);

        // Add it to our dictionary!
        loads[l.node] = l;
    }

    return loads;
}