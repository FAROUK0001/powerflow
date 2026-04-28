#include "core/parsers/phase_config_parser.hpp"
#include "core/parsers/parser_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::unordered_map<std::string, PhaseConfig> PhaseConfigParser::parse(const std::string& filepath) {
    std::unordered_map<std::string, PhaseConfig> configs;
    std::ifstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);

    std::string line;
    std::getline(file, line); // skip header
    int line_num = 1;

    while (std::getline(file, line)) {
        ++line_num;
        if (line.empty() || line.find_first_not_of("\r\n\t ") == std::string::npos) continue;

        std::stringstream ss(line);
        std::string piece, config_id, phase_row_str;

        try {
            std::getline(ss, config_id, ',');       parser_utils::trim_token(config_id);
            std::getline(ss, phase_row_str, ',');   parser_utils::trim_token(phase_row_str);

            if (config_id.empty())
                throw std::runtime_error("missing config_id");
            if (phase_row_str.empty())
                throw std::runtime_error("missing phase row label");

            int row = 0;
            if (phase_row_str == "B") row = 1;
            else if (phase_row_str == "C") row = 2;

            if (configs.find(config_id) == configs.end()) {
                configs[config_id] = PhaseConfig();
                configs[config_id].config_id = config_id;
            }

            PhaseConfig& config = configs[config_id];

            // Read 3 (R, X) pairs for Z_matrix
            for (int col = 0; col < 3; col++) {
                std::getline(ss, piece, ','); parser_utils::trim_token(piece);
                if (piece.empty()) throw std::runtime_error("missing Z real part at col " + std::to_string(col));
                double r = std::stod(piece);

                std::getline(ss, piece, ','); parser_utils::trim_token(piece);
                if (piece.empty()) throw std::runtime_error("missing Z imag part at col " + std::to_string(col));
                double x = std::stod(piece);

                config.Z_matrix(row, col) = std::complex<double>(r, x);
            }

            // Read 3 B values for B_matrix
            for (int col = 0; col < 3; col++) {
                std::getline(ss, piece, ','); parser_utils::trim_token(piece);
                if (piece.empty()) throw std::runtime_error("missing B value at col " + std::to_string(col));
                double b = std::stod(piece);
                config.B_matrix(row, col) = std::complex<double>(0.0, b);
            }
        } catch (const std::exception& e) {
            std::cerr << "phase_config_parser: skipping malformed line " << line_num
                      << " [" << line << "]: " << e.what() << '\n';
        }
    }
    return configs;
}