#include "core/parsers/distributed_load_parser.hpp"
#include "core/parsers/parser_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::vector<DistributedLoad> DistributedLoadParser::parse(const std::string& filename) {
    std::vector<DistributedLoad> loads;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Distributed Load file: " + filename);
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
        DistributedLoad dl;

        try {
            std::getline(ss, dl.node_a, ','); parser_utils::trim_token(dl.node_a);
            std::getline(ss, dl.node_b, ','); parser_utils::trim_token(dl.node_b);
            std::getline(ss, dl.model, ',');  parser_utils::trim_token(dl.model);

            if (dl.node_a.empty() || dl.node_b.empty())
                throw std::runtime_error("missing node name");

            for (int phase = 0; phase < 3; phase++) {
                std::getline(ss, word, ','); parser_utils::trim_token(word);
                if (word.empty()) throw std::runtime_error("missing kW for phase " + std::to_string(phase));
                dl.kw[phase] = std::stod(word);

                std::getline(ss, word, ','); parser_utils::trim_token(word);
                if (word.empty()) throw std::runtime_error("missing kVAr for phase " + std::to_string(phase));
                dl.kvar[phase] = std::stod(word);
            }

            loads.push_back(dl);
        } catch (const std::exception& e) {
            std::cerr << "distributed_load_parser: skipping malformed line " << line_num
                      << " [" << line << "]: " << e.what() << '\n';
        }
    }

    return loads;
}