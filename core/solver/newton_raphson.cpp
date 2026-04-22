#include "core/solver/newton_raphson.hpp"
#include "core/solver/jacobian_builder.hpp"
#include <cmath>

SolverResult NewtonRaphson::solve(
    const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
    const std::unordered_map<std::string, Load>& spot_loads,
    const std::vector<DistributedLoad>& dist_loads,
    const std::unordered_map<std::string, int>& node_to_index
) {
    const int num_nodes = static_cast<int>(node_to_index.size());
    const std::complex<double> zero(0.0, 0.0);

    // 1. GENERATE THE FLAT START VOLTAGES (V)
    const ComplexVector3 zero_vector(3, zero);
    std::vector<ComplexVector3> V(num_nodes, zero_vector);
    const double v_mag = 24900.0 / std::sqrt(3.0);

    const std::complex<double> vA(v_mag * std::cos(0.0), v_mag * std::sin(0.0));
    const std::complex<double> vB(v_mag * std::cos(-120.0 * (M_PI / 180.0)), v_mag * std::sin(-120.0 * (M_PI / 180.0)));
    const std::complex<double> vC(v_mag * std::cos(120.0 * (M_PI / 180.0)), v_mag * std::sin(120.0 * (M_PI / 180.0)));

    for (int i = 0; i < num_nodes; i++) {
        V[i][0] = vA; V[i][1] = vB; V[i][2] = vC;
    }

    // 2. GENERATE THE LOAD INJECTION VECTOR (S_spec)
    std::vector<ComplexVector3> S_spec(num_nodes, zero_vector);

    for (const auto& [node_name, load] : spot_loads) {
        const auto node_it = node_to_index.find(node_name);
        if (node_it == node_to_index.end()) continue;
        const int i = node_it->second;

        for (int phase = 0; phase < 3; phase++) {
            S_spec[i][phase] += std::complex<double>(-1.0 * load.kw[phase] * 1000.0, -1.0 * load.kvar[phase] * 1000.0);
        }
    }

    for (const auto& d_load : dist_loads) {
        const auto node_a_it = node_to_index.find(d_load.node_a);
        if (node_a_it == node_to_index.end()) continue;
        const auto node_b_it = node_to_index.find(d_load.node_b);
        if (node_b_it == node_to_index.end()) continue;
        const int node_a = node_a_it->second;
        const int node_b = node_b_it->second;

        for (int phase = 0; phase < 3; phase++) {
            const double p_half = -0.5 * (d_load.kw[phase] * 1000.0);
            const double q_half = -0.5 * (d_load.kvar[phase] * 1000.0);
            S_spec[node_a][phase] += std::complex<double>(p_half, q_half);
            S_spec[node_b][phase] += std::complex<double>(p_half, q_half);
        }
    }

    // 3. CALCULATE ACTUAL POWER FLOW (S_calc)
    std::vector<ComplexVector3> S_calc(num_nodes, zero_vector);
    const auto& y_values = ybus.get_values();
    const auto& y_cols   = ybus.get_col_indices();
    const auto& y_rows   = ybus.get_row_ptr();

    for (int i = 0; i < num_nodes; i++) {
        const ComplexVector3& v_i = V[i];
        ComplexVector3& s_i = S_calc[i];
        const int row_start = y_rows[i];
        const int row_end = y_rows[i + 1];
        for (int p_i = 0; p_i < 3; p_i++) {
            std::complex<double> sum_YV(0, 0);
            for (int ptr = row_start; ptr < row_end; ptr++) {
                const int j = y_cols[ptr];
                const ComplexMatrix3x3& Y_block = y_values[ptr];
                const ComplexVector3& v_j = V[j];
                for (int p_j = 0; p_j < 3; p_j++) {
                    sum_YV += Y_block(p_i, p_j) * v_j[p_j];
                }
            }
            s_i[p_i] = v_i[p_i] * std::conj(sum_YV);
        }
    }

    // 4. CALCULATE MISMATCH
    std::vector<ComplexVector3> mismatch(num_nodes, zero_vector);
    for (int i = 1; i < num_nodes; i++) { // Skip Substation (Node 0)
        for (int p = 0; p < 3; p++) {
            mismatch[i][p] = S_spec[i][p] - S_calc[i][p];
        }
    }
    RealMatrix J = JacobianBuilder::build(ybus, V, S_calc, num_nodes);
    // Return all data to the caller!
    return { V, S_spec, S_calc, mismatch ,J};
}
