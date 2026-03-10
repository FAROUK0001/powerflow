#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include "core/parsers/distributed_load_parser.hpp"
#include "core/parsers/regulator_parser.hpp"
#include "core/parsers/capacitor_parser.hpp"
int main() {

// Load the new files!
std::string dist_load_file = "../feeder34/data/distributed_load_data.csv";
std::string reg_file = "../feeder34/data/regulator_data.csv";
std::string cap_file = "../feeder34/data/cap_data.csv"; // Note: filename might be different for you!

auto dist_loads = DistributedLoadParser::parse(dist_load_file);
auto regulators = RegulatorParser::parse(reg_file);
auto capacitors = CapacitorParser::parse(cap_file);

// Print the results to prove it works!
std::cout << "🛣️  Total Distributed Loads: " << dist_loads.size() << "\n";
std::cout << "🎛️  Total Regulator Phases: " << regulators.size() << "\n";
std::cout << "🔋 Total Capacitor Banks: " << capacitors.size() << "\n";

if (capacitors.contains("844")) {
    std::cout << "   Node 844 is injecting " << capacitors["844"].kvar[0] << " kVAr on Phase A!\n";
}
}