#include "core/parsers/load_parser.hpp"
#include "core/parsers/parser_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::unordered_map<std::string, Load> LoadParser::parse(const std::string& filename) {
    std::unordered_map<std::string, Load> loads;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open Load file: " + filename);
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
        Load l;

        try {
            std::getline(ss, l.node, ',');  parser_utils::trim_token(l.node);
            std::getline(ss, l.model, ','); parser_utils::trim_token(l.model);

            if (l.node.empty()) throw std::runtime_error("missing node name");

            // Phase 1 (kW and kVAr)
            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing Ph1 kW");
            l.kw[0] = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing Ph1 kVAr");
            l.kvar[0] = std::stod(word);

            // Phase 2 (kW and kVAr)
            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing Ph2 kW");
            l.kw[1] = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing Ph2 kVAr");
            l.kvar[1] = std::stod(word);

            // Phase 3 (kW and kVAr)
            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing Ph3 kW");
            l.kw[2] = std::stod(word);

            std::getline(ss, word, ','); parser_utils::trim_token(word);
            if (word.empty()) throw std::runtime_error("missing Ph3 kVAr");
            l.kvar[2] = std::stod(word);

            loads[l.node] = l;
        } catch (const std::exception& e) {
            std::cerr << "load_parser: skipping malformed line " << line_num
                      << " [" << line << "]: " << e.what() << '\n';
        }
    }

    return loads;
}