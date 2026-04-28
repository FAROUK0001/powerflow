#pragma once
#include <string>

// Join a directory root and a file name into a single path.
// Handles a trailing slash on root and an empty root gracefully.
inline std::string data_path(const std::string& root, const std::string& file) {
    if (root.empty()) return file;
    if (root.back() == '/') return root + file;
    return root + "/" + file;
}
