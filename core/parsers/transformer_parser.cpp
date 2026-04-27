#include "core/parsers/transformer_parser.hpp"
#include "core/parsers/parser_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::unordered_map<std::string, Transformer> TransformerParser::parse(const std::string& filename) {
    std::unordered_map<std::string, Transformer> transformers;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Transformer file: " + filename);
    }

    std::string line, word;
    std::getline(file, line); // skip header
    int line_num = 1;

    while (std::getline(file, line)) {
        ++line_num;
        if (line.empty() || line.find_first_not_of("\r\n\t ") == std::string::npos) continue;

        std::stringstream ss(line);
        Transformer tx;

        try {
            std::getline(ss, tx.name, ','); parser_utils::trim_token(tx.name);
            if (tx.name.empty()) throw std::runtime_error("missing transformer name");

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing kVA");
            tx.kva = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing kV_high");
            tx.kv_high = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing kV_low");
            tx.kv_low = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing R%");
            tx.r_percent = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing X%");
            tx.x_percent = std::stod(word);

            transformers[tx.name] = tx;
        } catch (const std::exception& e) {
            std::cerr << "transformer_parser: skipping malformed line " << line_num
                      << " [" << line << "]: " << e.what() << '\n';
        }
    }

    return transformers;
}