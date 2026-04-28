#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <complex>
#include <iomanip>

// Include your parsers and models
#include "core/parsers/line_parser.hpp"
#include "core/parsers/phase_config_parser.hpp"
#include "core/parsers/capacitor_parser.hpp"
#include "core/parsers/transformer_parser.hpp"
#include "core/models/ybus_builder.hpp"
#include "core/utils/path_utils.hpp"

int main(int argc, char** argv) {
    // Data directory: use argv[1] if supplied, otherwise fall back to the
    // path embedded at build time via POWERFLOW_DATA_DIR.
    const std::string data_dir = (argc >= 2) ? argv[1] : POWERFLOW_DATA_DIR;

    std::cout << "--- BUILDING Y-BUS FOR FULL CONSOLE PRINT ---\n";

    try {
        // 1. Read the exact same CSVs as main
        auto branches = LineParser::parse(data_path(data_dir, "line_data.csv"));
        auto configs = PhaseConfigParser::parse(data_path(data_dir, "line_matrices.csv"));
        auto capacitors = CapacitorParser::parse(data_path(data_dir, "cap_data.csv"));
        auto transformers = TransformerParser::parse(data_path(data_dir, "transformer_data.csv"));

        // 2. Build the Node Dictionary AND a Reverse Dictionary!
        std::unordered_map<std::string, int> node_to_index;
        std::unordered_map<int, std::string> index_to_node; // <--- The Reverse Map!
        int current_index = 0;

        for (const auto& branch : branches) {
            if (!node_to_index.contains(branch.node_a)) {
                node_to_index[branch.node_a] = current_index;
                index_to_node[current_index] = branch.node_a;
                current_index++;
            }
            if (!node_to_index.contains(branch.node_b)) {
                node_to_index[branch.node_b] = current_index;
                index_to_node[current_index] = branch.node_b;
                current_index++;
            }
        }

        int num_nodes = node_to_index.size();

        // 3. Build the Map-Based Y-Bus
        const double base_kv = 24.9; // IEEE 34-bus nominal line-to-line voltage (kV)
        auto ybus_map = YBusBuilder::build_ybus_map(branches, configs, capacitors, transformers, node_to_index, base_kv);

        // 4. PRINT EVERY SINGLE SLOT TO CONSOLE
        std::cout << "\n====================================================\n";
        std::cout << "🖨️ PRINTING 34x34 Y-BUS (ALL 1156 BLOCKS)\n";
        std::cout << "====================================================\n\n";

        // Formatting for neat columns
        std::cout << std::fixed << std::setprecision(5);

        for (int r = 0; r < num_nodes; r++) {
            for (int c = 0; c < num_nodes; c++) {

                // Use the reverse map to get the ACTUAL bus names!
                std::string bus_r = index_to_node[r];
                std::string bus_c = index_to_node[c];

                std::cout << "--- Row Bus " << bus_r << ", Col Bus " << bus_c << " ---\n";

                // Check if this block actually has data
                if (ybus_map.contains(r) && ybus_map[r].contains(c)) {
                    ComplexMatrix3x3 block = ybus_map[r][c];

                    // Print the 3x3 matrix
                    for (int i = 0; i < 3; i++) {
                        std::cout << "  [ ";
                        for (int j = 0; j < 3; j++) {
                            double real_val = block(i, j).real();
                            double imag_val = block(i, j).imag();

                            // Format: Real + j*Imag
                            std::cout << std::setw(10) << real_val;
                            if (imag_val >= 0) std::cout << " + j" << std::left << std::setw(8) << imag_val << std::right;
                            else               std::cout << " - j" << std::left << std::setw(8) << -imag_val << std::right;

                            if (j < 2) std::cout << " | ";
                        }
                        std::cout << " ]\n";
                    }
                } else {
                    // Empty slot! Print 3x3 Zeros.
                    for (int i = 0; i < 3; i++) {
                        std::cout << "  [ "
                                  << std::setw(10) << 0.00000 << " + j" << std::left << std::setw(8) << 0.00000 << std::right << " | "
                                  << std::setw(10) << 0.00000 << " + j" << std::left << std::setw(8) << 0.00000 << std::right << " | "
                                  << std::setw(10) << 0.00000 << " + j" << std::left << std::setw(8) << 0.00000 << std::right << " ]\n";
                    }
                }
                std::cout << "\n";
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ ERROR: " << e.what() << "\n";
    }

    return 0;
}