#include "core/parsers/phase_config_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::unordered_map<std::string, PhaseConfig> PhaseConfigParser::parse(const std::string& filepath) {
    std::unordered_map<std::string, PhaseConfig> configs;
    std::ifstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string piece, config_id, phase_row_str;

        try {
            std::getline(ss, config_id, ',');
            std::getline(ss, phase_row_str, ',');

            int row = 0;
            if (phase_row_str == "B") row = 1;
            if (phase_row_str == "C") row = 2;

            if (configs.find(config_id) == configs.end()) {
                configs[config_id] = PhaseConfig();
                configs[config_id].config_id = config_id;
            }

            PhaseConfig& config = configs[config_id];

            for (int col = 0; col < 3; col++) {
                std::getline(ss, piece, ','); double r = std::stod(piece);
                std::getline(ss, piece, ','); double x = std::stod(piece);
                config.Z_matrix(row, col) = std::complex<double>(r, x);
            }
            for (int col = 0; col < 3; col++) {
                std::getline(ss, piece, ','); double b = std::stod(piece);
                config.B_matrix(row, col) = std::complex<double>(0.0, b);
            }
        } catch (const std::exception& e) {
            std::cerr << "phase_config_parser: skipping malformed line: " << e.what() << '\n';
        }
    }
    return configs;
}