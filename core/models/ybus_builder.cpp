#include "core/models/ybus_builder.hpp"
#include <iostream>

std::unordered_map<int, std::unordered_map<int, ComplexMatrix3x3>>
YBusBuilder::build_ybus_map(
    const std::vector<Branch>& branches,
    const std::unordered_map<std::string, PhaseConfig>& configs,
    const std::unordered_map<std::string, Capacitor>& capacitors,
    const std::unordered_map<std::string, Transformer>& transformers,
    const std::unordered_map<std::string, int>& node_to_index
)
{
    std::unordered_map<int, std::unordered_map<int, ComplexMatrix3x3>> ybus;
    ybus.reserve(node_to_index.size());

    for (const auto& branch : branches) {
        const int i = node_to_index.at(branch.node_a);
        const int j = node_to_index.at(branch.node_b);

        auto& row_i = ybus[i];
        auto& row_j = ybus[j];

        auto yii_it = row_i.try_emplace(i, ComplexMatrix3x3(3, 3)).first;
        auto yjj_it = row_j.try_emplace(j, ComplexMatrix3x3(3, 3)).first;
        auto yij_it = row_i.try_emplace(j, ComplexMatrix3x3(3, 3)).first;
        auto yji_it = row_j.try_emplace(i, ComplexMatrix3x3(3, 3)).first;

        // ==============================================================
        // SCENARIO 1: IT IS A NORMAL POWER LINE
        // ==============================================================
        if (const auto config_it = configs.find(branch.config_id); config_it != configs.end()) {
            ComplexMatrix3x3 Z_line = config_it->second.Z_matrix;
            const double length_in_miles = branch.length_ft / 5280.0;

            for(int r = 0; r < 3; r++) {
                for(int c = 0; c < 3; c++) {
                    Z_line(r,c) = Z_line(r,c) * length_in_miles;
                }
            }

            ComplexMatrix3x3 Y_line = Z_line.inverse();
            ComplexMatrix3x3 neg_Y_line(3, 3);
            for(int r=0; r<3; r++) {
                for(int c=0; c<3; c++) {
                    neg_Y_line(r,c) = Y_line(r,c) * std::complex<double>(-1, 0);
                }
            }

            yii_it->second = yii_it->second + Y_line;
            yjj_it->second = yjj_it->second + Y_line;
            yij_it->second = yij_it->second + neg_Y_line;
            yji_it->second = yji_it->second + neg_Y_line;
        }
        // ==============================================================
        // SCENARIO 2: IT IS A TRANSFORMER
        // ==============================================================
        else if (const auto tx_it = transformers.find(branch.config_id); tx_it != transformers.end()) {
            const Transformer& tx = tx_it->second;

            // 1. Calculate Base Ohms on the High Side
            double z_base = (tx.kv_high * tx.kv_high * 1000.0) / tx.kva;

            // 2. Convert % Impedance to Actual Ohms
            std::complex<double> z_actual(
                (tx.r_percent / 100.0) * z_base,
                (tx.x_percent / 100.0) * z_base
            );

            // 3. Convert Actual Ohms to Admittance
            std::complex<double> y_H = 1.0 / z_actual;
            double a = tx.kv_high / tx.kv_low;

            ComplexMatrix3x3 Y11(3,3), Y22(3,3), Y12(3,3), Y21(3,3);

            for (int p = 0; p < 3; p++) {
                Y11(p, p) = y_H;
                Y22(p, p) = y_H * (a * a);
                Y12(p, p) = -y_H * a;
                Y21(p, p) = -y_H * a;
            }

            yii_it->second = yii_it->second + Y11;
            yjj_it->second = yjj_it->second + Y22;
            yij_it->second = yij_it->second + Y12;
            yji_it->second = yji_it->second + Y21;
        }
        else {
            std::cerr << "Warning: Config " << branch.config_id << " missing!\n";
        }
    }

    // ==============================================================
    // 5. INJECT CAPACITORS (Shunt Admittance)
    // ==============================================================
    for (const auto& cap_pair : capacitors) {
        const std::string& node_name = cap_pair.first;
        const Capacitor& cap = cap_pair.second;

        const auto node_it = node_to_index.find(node_name);
        if (node_it == node_to_index.end()) continue;
        const int i = node_it->second;

        ComplexMatrix3x3 Y_cap(3, 3);
        double base_kV = 24.9;
        double z_base = (base_kV * base_kV) / 1000.0;

        for (int phase = 0; phase < 3; phase++) {
            if (cap.kvar[phase] > 0) {
                double b_siemens = (cap.kvar[phase] / 1000.0) / z_base;
                Y_cap(phase, phase) = std::complex<double>(0.0, b_siemens);
            }
        }

        auto& row_i = ybus[i];
        auto yii_it = row_i.try_emplace(i, ComplexMatrix3x3(3, 3)).first;
        yii_it->second = yii_it->second + Y_cap;
    }

    return ybus;
}
