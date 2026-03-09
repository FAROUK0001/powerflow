#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <complex>

// Make sure these include paths match your actual file names!
#include "core/parsers/line_parser.hpp"
#include "core/parsers/phase_config_parser.hpp"
#include "core/models/ybus_builder.hpp"
#include "core/linalg/matrix_sparse_csr.hpp" // Your awesome CSR class!

int main() {
    std::cout << "--- IEEE 34-Bus: Full Y-Bus CSR Test ---\n\n";

    try {
        // 1. Read the Data (Using your exact requested paths)
        std::string line_file = "../feeder34/data/line_data.csv";
        std::string config_file = "../feeder34/data/line_matrices.csv";

        std::vector<Branch> branches = LineParser::parse(line_file);
        std::unordered_map<std::string, PhaseConfig> configs = PhaseConfigParser::parse(config_file);

        // 2. Build the Node Dictionary
        std::unordered_map<std::string, int> node_to_index;
        int current_index = 0;
        for (const auto& branch : branches) {
            if (!node_to_index.contains(branch.node_a)) node_to_index[branch.node_a] = current_index++;
            if (!node_to_index.contains(branch.node_b)) node_to_index[branch.node_b] = current_index++;
        }

        int num_nodes = node_to_index.size();
        std::cout << "✅ Mapped " << num_nodes << " unique nodes.\n";

        // 3. Build the Map-Based Y-Bus
        std::cout << "⚙️ Building Map-based Y-Bus...\n";
        auto ybus_map = YBusBuilder::build_ybus_map(branches, configs, node_to_index);

        // 4. Convert to your highly optimized Sparse CSR Matrix!
        std::cout << "🗜️ Compressing into Sparse CSR format...\n";

        // Initialize your sparse matrix
        MatrixSparseCSR<ComplexMatrix3x3> final_ybus(num_nodes, num_nodes);

        int non_zero_count = 0;

        // Dump the dictionary into your Triplet builder
        for (const auto& row_pair : ybus_map) {
            int row = row_pair.first;
            for (const auto& col_pair : row_pair.second) {
                int col = col_pair.first;
                ComplexMatrix3x3 matrix_block = col_pair.second;

                // Use YOUR awesome add_value function!
                final_ybus.add_value(row, col, matrix_block);
                non_zero_count++;
            }
        }

        // Call your magic sort and build function!
        final_ybus.build_csr();

        // 5. Victory Printout!
        std::cout << "✅ Final Y-Bus CSR Matrix Built Successfully!\n";
        std::cout << "   - Grid Size: " << num_nodes << " x " << num_nodes << " nodes\n";
        std::cout << "   - Total possible 3x3 blocks: " << (num_nodes * num_nodes) << "\n";
        std::cout << "   - Actual Non-Zero blocks stored: " << non_zero_count << "\n";
        std::cout << "   - Memory slots saved: " << (num_nodes * num_nodes) - non_zero_count << " blocks ignored!\n\n";

    } catch (const std::exception& e) {
        std::cerr << "❌ ERROR: " << e.what() << "\n";
    }

    return 0;
}