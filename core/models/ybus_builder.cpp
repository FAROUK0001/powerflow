#include "core/models/ybus_builder.hpp"
#include <iostream>

std::unordered_map<int, std::unordered_map<int, ComplexMatrix3x3>>
YBusBuilder::build_ybus_map(
    const std::vector<Branch>& branches,
    const std::unordered_map<std::string, PhaseConfig>& configs,
    const std::unordered_map<std::string, int>& node_to_index
) {
    // 1. Create our "Dictionary of Dictionaries" to hold the Y-Bus
    // ybus[row][col] = 3x3 Matrix
    std::unordered_map<int, std::unordered_map<int, ComplexMatrix3x3>> ybus;

    // 2. Loop through every single power line
    for (const auto& branch : branches) {

        // Find the integer indices for the From and To nodes
        int i = node_to_index.at(branch.node_a);
        int j = node_to_index.at(branch.node_b);

        // Look up the 3x3 Impedance from the configuration
        if (configs.find(branch.config_id) == configs.end()) {
            std::cerr << "Warning: Config " << branch.config_id << " missing!\n";
            continue;
        }

        ComplexMatrix3x3 Z_line = configs.at(branch.config_id).Z_matrix;

        // Optional: Multiply Z by the length of the line! (Usually Z is per-mile or per-ft)
        // If your data requires it, you would do it here. For now, we assume Z is total impedance.

        // 3. INVERT IT! Calculate the Admittance of the line
        ComplexMatrix3x3 Y_line = Z_line.inverse();

        // 4. THE Y-BUS RULES
        // Rule A: Diagonal Y_ii += Y_line
        if (ybus[i].find(i) == ybus[i].end()) ybus[i][i] = ComplexMatrix3x3(3, 3);
        ybus[i][i] = ybus[i][i] + Y_line;

        // Rule B: Diagonal Y_jj += Y_line
        if (ybus[j].find(j) == ybus[j].end()) ybus[j][j] = ComplexMatrix3x3(3, 3);
        ybus[j][j] = ybus[j][j] + Y_line;

        // Rule C: Off-Diagonal Y_ij -= Y_line
        if (ybus[i].find(j) == ybus[i].end()) ybus[i][j] = ComplexMatrix3x3(3, 3);
        // We have to subtract it, so we multiply by -1
        ComplexMatrix3x3 neg_Y_line(3, 3);
        for(int r=0; r<3; r++) for(int c=0; c<3; c++) neg_Y_line(r,c) = Y_line(r,c) * std::complex<double>(-1, 0);
        ybus[i][j] = ybus[i][j] + neg_Y_line;

        // Rule D: Off-Diagonal Y_ji -= Y_line
        if (ybus[j].find(i) == ybus[j].end()) ybus[j][i] = ComplexMatrix3x3(3, 3);
        ybus[j][i] = ybus[j][i] + neg_Y_line;
    }

    return ybus;
}