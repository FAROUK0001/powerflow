#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/models/ybus_builder.hpp"
#include "core/parsers/capacitor_parser.hpp"
#include "core/parsers/distributed_load_parser.hpp"
#include "core/parsers/line_parser.hpp"
#include "core/parsers/load_parser.hpp"
#include "core/parsers/phase_config_parser.hpp"
#include "core/parsers/transformer_parser.hpp"
#include "core/solver/newton_raphson.hpp"

namespace {
std::string join_data_path(const std::string& root, const std::string& file_name) {
    if (root.empty()) {
        return file_name;
    }
    if (root.back() == '/') {
        return root + file_name;
    }
    return root + "/" + file_name;
}

using Clock = std::chrono::steady_clock;

double milliseconds_between(const Clock::time_point start, const Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

double average_ms(const std::vector<double>& samples) {
    if (samples.empty()) {
        return 0.0;
    }
    return std::accumulate(samples.begin(), samples.end(), 0.0) / static_cast<double>(samples.size());
}
} // namespace

int main(int argc, char** argv) {
    try {
        int warmup_runs = 1;
        int measured_runs = 3;
        std::string data_root = "../feeder34/data";

        if (argc >= 2) {
            warmup_runs = std::max(0, std::atoi(argv[1]));
        }
        if (argc >= 3) {
            measured_runs = std::max(1, std::atoi(argv[2]));
        }
        if (argc >= 4) {
            data_root = argv[3];
        }

        const auto branches = LineParser::parse(join_data_path(data_root, "line_data.csv"));
        const auto configs = PhaseConfigParser::parse(join_data_path(data_root, "line_matrices.csv"));
        const auto spot_loads = LoadParser::parse(join_data_path(data_root, "spot_load_data.csv"));
        const auto dist_loads = DistributedLoadParser::parse(join_data_path(data_root, "distributed_load_data.csv"));
        const auto capacitors = CapacitorParser::parse(join_data_path(data_root, "cap_data.csv"));
        const auto transformers = TransformerParser::parse(join_data_path(data_root, "transformer_data.csv"));

        std::unordered_map<std::string, int> node_to_index;
        node_to_index.reserve(branches.size() * 2);
        int current_index = 0;
        for (const auto& branch : branches) {
            if (!node_to_index.contains(branch.node_a)) {
                node_to_index[branch.node_a] = current_index++;
            }
            if (!node_to_index.contains(branch.node_b)) {
                node_to_index[branch.node_b] = current_index++;
            }
        }

        const int num_nodes = static_cast<int>(node_to_index.size());
        if (num_nodes == 0) {
            std::cerr << "No nodes parsed from input data.\n";
            return 1;
        }

        std::vector<double> ybus_samples_ms;
        std::vector<double> solve_samples_ms;
        ybus_samples_ms.reserve(static_cast<std::size_t>(measured_runs));
        solve_samples_ms.reserve(static_cast<std::size_t>(measured_runs));

        int csr_nnz_checksum = 0;

        auto run_ybus_once = [&]() {
            auto ybus_map = YBusBuilder::build_ybus_map(branches, configs, capacitors, transformers, node_to_index);
            MatrixSparseCSR<ComplexMatrix3x3> final_ybus(num_nodes, num_nodes);
            for (const auto& row_pair : ybus_map) {
                for (const auto& col_pair : row_pair.second) {
                    final_ybus.add_value(row_pair.first, col_pair.first, col_pair.second);
                }
            }
            final_ybus.build_csr();
            csr_nnz_checksum += static_cast<int>(final_ybus.get_values().size());
            return final_ybus;
        };

        MatrixSparseCSR<ComplexMatrix3x3> benchmark_ybus = run_ybus_once();

        for (int i = 0; i < warmup_runs; ++i) {
            benchmark_ybus = run_ybus_once();
            const SolverResult warmup_result = NewtonRaphson::solve(benchmark_ybus, spot_loads, dist_loads, node_to_index);
            csr_nnz_checksum += static_cast<int>(warmup_result.J.size());
        }

        for (int run = 0; run < measured_runs; ++run) {
            const auto ybus_start = Clock::now();
            benchmark_ybus = run_ybus_once();
            const auto ybus_end = Clock::now();
            const double ybus_ms = milliseconds_between(ybus_start, ybus_end);
            ybus_samples_ms.push_back(ybus_ms);

            const auto solve_start = Clock::now();
            const SolverResult solve_result = NewtonRaphson::solve(benchmark_ybus, spot_loads, dist_loads, node_to_index);
            const auto solve_end = Clock::now();
            const double solve_ms = milliseconds_between(solve_start, solve_end);
            solve_samples_ms.push_back(solve_ms);
            csr_nnz_checksum += static_cast<int>(solve_result.J.size());

            std::cout << "run=" << (run + 1)
                      << " ybus_ms=" << std::fixed << std::setprecision(3) << ybus_ms
                      << " solve_ms=" << std::fixed << std::setprecision(3) << solve_ms
                      << "\n";
        }

        std::cout << "avg_ybus_ms=" << std::fixed << std::setprecision(3) << average_ms(ybus_samples_ms) << "\n";
        std::cout << "avg_solve_ms=" << std::fixed << std::setprecision(3) << average_ms(solve_samples_ms) << "\n";
        std::cout << "checksum=" << csr_nnz_checksum << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Benchmark failed: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
