#pragma once
#include <string>

namespace parser_utils {

// Remove leading and trailing whitespace and carriage returns from a token.
// This makes CSV parsing robust to Windows (CRLF) line endings.
inline void trim_token(std::string& s) {
    // Strip trailing whitespace / CR
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
                           s.back() == ' '  || s.back() == '\t')) {
        s.pop_back();
    }
    // Strip leading whitespace
    const auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        s.clear();
    } else if (start > 0) {
        s = s.substr(start);
    }
}

} // namespace parser_utils
