#include "core/parsers/regulator_parser.hpp"
#include "core/parsers/parser_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::vector<Regulator> RegulatorParser::parse(const std::string& filename) {
    std::vector<Regulator> regulators;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Regulator file: " + filename);
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
        Regulator reg;

        try {
            std::getline(ss, reg.id, ',');        parser_utils::trim_token(reg.id);
            std::getline(ss, reg.from_node, ','); parser_utils::trim_token(reg.from_node);
            std::getline(ss, reg.to_node, ',');   parser_utils::trim_token(reg.to_node);

            if (reg.id.empty()) throw std::runtime_error("missing regulator id");

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing phase");
            reg.phase = std::stoi(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing v_hold");
            reg.v_hold = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing r_volt");
            reg.r_volt = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing x_volt");
            reg.x_volt = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing pt_ratio");
            reg.pt_ratio = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing ct_rate");
            reg.ct_rate = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing bandwidth");
            reg.bandwidth = std::stod(word);

            regulators.push_back(reg);
        } catch (const std::exception& e) {
            std::cerr << "regulator_parser: skipping malformed line " << line_num
                      << " [" << line << "]: " << e.what() << '\n';
        }
    }

    return regulators;
}