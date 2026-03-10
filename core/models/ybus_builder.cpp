#include "core/models/ybus_builder.hpp"
#include <iostream>

std::unordered_map<int, std::unordered_map<int, ComplexMatrix3x3>>
YBusBuilder::build_ybus_map(
    const std::vector<Branch>& branches,
    const std::unordered_map<std::string, PhaseConfig>& configs,
    const std::unordered_map<std::string, Transformer>& transformers, // <--- NEW ARGUMENT
    const std::unordered_map<std::string, int>& node_to_index
) {
    std::unordered_map<int, std::unordered_map<int, ComplexMatrix3x3>> ybus;

    for (const auto& branch : branches) {
        int i = node_to_index.at(branch.node_a);
        int j = node_to_index.at(branch.node_b);

        // Pre-initialize empty 3x3 matrices so we can safely add to them later
        if (!ybus[i].contains(i)) ybus[i][i] = ComplexMatrix3x3(3, 3);
        if (!ybus[j].contains(j)) ybus[j][j] = ComplexMatrix3x3(3, 3);
        if (!ybus[i].contains(j)) ybus[i][j] = ComplexMatrix3x3(3, 3);
        if (!ybus[j].contains(i)) ybus[j][i] = ComplexMatrix3x3(3, 3);

        // ==============================================================
        // SCENARIO 1: IT IS A NORMAL POWER LINE
        // ==============================================================
        if (configs.contains(branch.config_id)) {

            ComplexMatrix3x3 Z_line = configs.at(branch.config_id).Z_matrix;
            // Convert length from feet to miles
            double length_in_miles = branch.length_ft / 5280.0;

            for(int r = 0; r < 3; r++) {
                for(int c = 0; c < 3; c++) {
                    // Multiply the Ohms/mile by the actual distance!
                    Z_line(r,c) = Z_line(r,c) * length_in_miles;
                }
            }
            // ------------------------------------------
            ComplexMatrix3x3 Y_line = Z_line.inverse();

            ComplexMatrix3x3 neg_Y_line(3, 3);
            for(int r=0; r<3; r++) {
                for(int c=0; c<3; c++) {
                    neg_Y_line(r,c) = Y_line(r,c) * std::complex<double>(-1, 0);
                }
            }

            ybus[i][i] = ybus[i][i] + Y_line;
            ybus[j][j] = ybus[j][j] + Y_line;
            ybus[i][j] = ybus[i][j] + neg_Y_line;
            ybus[j][i] = ybus[j][i] + neg_Y_line;
        }
        // ==============================================================
        // SCENARIO 2: IT IS A TRANSFORMER (THE NEW LOGIC)
        // ==============================================================
        else if (transformers.find(branch.config_id) != transformers.end()) {

            const Transformer& tx = transformers.at(branch.config_id);

            // 1. Calculate Base Ohms on the High Side
            double z_base = (tx.kv_high * tx.kv_high * 1000.0) / tx.kva;

            // 2. Convert % Impedance to Actual Ohms (Z = R + jX)
            std::complex<double> z_actual(
                (tx.r_percent / 100.0) * z_base,
                (tx.x_percent / 100.0) * z_base
            );

            // 3. Convert Actual Ohms to Actual Siemens (Admittance)
            std::complex<double> y_H = 1.0 / z_actual;

            // 4. Calculate the Turns Ratio 'a'
            double a = tx.kv_high / tx.kv_low;

            // 5. Build the 4 distinct matrix blocks using 'a'
            ComplexMatrix3x3 Y11(3,3), Y22(3,3), Y12(3,3), Y21(3,3);

            // Transformers usually apply the exact same admittance to Phase A, B, and C
            for (int p = 0; p < 3; p++) {
                Y11(p, p) = y_H;             // High side node gets normal Y
                Y22(p, p) = y_H * (a * a);   // Low side node gets Y multiplied by a^2
                Y12(p, p) = -y_H * a;        // Off-diagonals get multiplied by 'a'
                Y21(p, p) = -y_H * a;        // Off-diagonals get multiplied by 'a'
            }

            ybus[i][i] = ybus[i][i] + Y11;
            ybus[j][j] = ybus[j][j] + Y22;
            ybus[i][j] = ybus[i][j] + Y12;
            ybus[j][i] = ybus[j][i] + Y21;
        }
        // ==============================================================
        // SCENARIO 3: MISSING CONFIGURATION
        // ==============================================================
        else {
            std::cerr << "Warning: Config " << branch.config_id << " missing!\n";
        }
    }

    return ybus;
}