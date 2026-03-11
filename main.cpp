#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <string>

// Parsers
#include "core/parsers/line_parser.hpp"
#include "core/parsers/phase_config_parser.hpp"
#include "core/parsers/load_parser.hpp"
#include "core/parsers/distributed_load_parser.hpp"
#include "core/parsers/capacitor_parser.hpp"
#include "core/parsers/transformer_parser.hpp"

// Math Engine
#include "core/models/ybus_builder.hpp"
#include "core/solver/newton_raphson.hpp"

int main() {
    std::cout << std::fixed << std::setprecision(5);
    std::cout << "--- IEEE 34-Bus: Newton-Raphson Engine ---\n\n";

    try {
        // 1. Read Data
        auto branches     = LineParser::parse("../feeder34/data/line_data.csv");
        auto configs      = PhaseConfigParser::parse("../feeder34/data/line_matrices.csv");
        auto spot_loads   = LoadParser::parse("../feeder34/data/spot_load_data.csv");
        auto dist_loads   = DistributedLoadParser::parse("../feeder34/data/distributed_load_data.csv");
        auto capacitors   = CapacitorParser::parse("../feeder34/data/cap_data.csv");
        auto transformers = TransformerParser::parse("../feeder34/data/transformer_data.csv");

        // 2. Build Dictionary
        std::unordered_map<std::string, int> node_to_index;
        int current_index = 0;
        for (const auto& branch : branches) {
            if (!node_to_index.contains(branch.node_a)) node_to_index[branch.node_a] = current_index++;
            if (!node_to_index.contains(branch.node_b)) node_to_index[branch.node_b] = current_index++;
        }
        int num_nodes = node_to_index.size();

        // 3. Build CSR Matrix
        auto ybus_map = YBusBuilder::build_ybus_map(branches, configs, capacitors, transformers, node_to_index);
        MatrixSparseCSR<ComplexMatrix3x3> final_ybus(num_nodes, num_nodes);
        for (const auto& row_pair : ybus_map) {
            for (const auto& col_pair : row_pair.second) {
                final_ybus.add_value(row_pair.first, col_pair.first, col_pair.second);
            }
        }
        final_ybus.build_csr();

        // 4. Run the Engine Step 1!
        std::cout << "🚀 Launching Mismatch Calculator & Jacobian Builder...\n";
        SolverResult results = NewtonRaphson::solve(final_ybus, spot_loads, dist_loads, node_to_index);

        // 5. TEST: Print the Mismatch for Node 844
        if (node_to_index.contains("844")) {
            int test_node = node_to_index.at("844");

            std::cout << "\n📊 --- NODE 844 DIAGNOSTICS ---\n";
            std::cout << "1. Target Load (S_spec):\n";
            std::cout << "   Ph A: " << results.S_spec[test_node][0].real() << " W + j" << results.S_spec[test_node][0].imag() << " VAr\n";
            std::cout << "\n2. Actual Power Flowing (S_calc):\n";
            std::cout << "   Ph A: " << results.S_calc[test_node][0].real() << " W + j" << results.S_calc[test_node][0].imag() << " VAr\n";
            std::cout << "\n3. Total Mismatch (Delta S):\n";
            std::cout << "   Ph A: " << results.mismatch[test_node][0].real() << " W + j" << results.mismatch[test_node][0].imag() << " VAr\n";
        }

        // 6. TEST: Inspect the Jacobian!
        std::cout << "\n🧮 --- JACOBIAN DIAGNOSTICS ---\n";
        std::cout << "Jacobian Rows: " << results.J.size() << "\n";
        std::cout << "Jacobian Cols: " << results.J[0].size() << "\n";

    } catch (const std::exception& e) {
        std::cerr << "❌ ERROR: " << e.what() << "\n";
    }

    return 0;
}