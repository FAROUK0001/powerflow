#include "core/parsers/distributed_load_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<DistributedLoad> DistributedLoadParser::parse(const std::string& filename) {
    std::vector<DistributedLoad> loads;
    std::ifstream file(filename);
    std::string line, word;

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Distributed Load file: " + filename);
    }

    // 1. THROWAWAY READ (Skips the Header!)
    std::getline(file, line);

    // 2. MAIN LOOP
    while (std::getline(file, line)) {
        if (line.empty() || line.find_first_not_of("\r\n\t ") == std::string::npos) {
            continue;
        }

        std::stringstream ss(line);
        DistributedLoad dl;

        // Columns 1, 2, & 3: Strings
        std::getline(ss, dl.node_a, ',');
        std::getline(ss, dl.node_b, ',');
        std::getline(ss, dl.model, ',');

        // Columns 4 & 5: Phase 1
        std::getline(ss, word, ','); dl.kw[0] = std::stod(word);
        std::getline(ss, word, ','); dl.kvar[0] = std::stod(word);

        // Columns 6 & 7: Phase 2
        std::getline(ss, word, ','); dl.kw[1] = std::stod(word);
        std::getline(ss, word, ','); dl.kvar[1] = std::stod(word);

        // Columns 8 & 9: Phase 3
        std::getline(ss, word, ','); dl.kw[2] = std::stod(word);
        std::getline(ss, word, ','); dl.kvar[2] = std::stod(word);

        loads.push_back(dl); // Add it to our list!
    }

    return loads;
}