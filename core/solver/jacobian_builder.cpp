#include "core/solver/jacobian_builder.hpp"
#include <iostream>
#include <cmath>

RealMatrix JacobianBuilder::build(
    const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
    const std::vector<ComplexVector3>& V,
    const std::vector<ComplexVector3>& S_calc,
    const StateIndex& state_index
) {
    std::cout << "   -> Assembling the Full Jacobian Matrix...\n";

    const int dimension = state_index.dimension;
    const int num_nodes = static_cast<int>(state_index.idx.size());
    RealMatrix J(static_cast<std::size_t>(dimension),
                 std::vector<double>(static_cast<std::size_t>(dimension), 0.0));

    const std::complex<double> J_MATH(0.0, 1.0);
    const std::complex<double> zero(0.0, 0.0);

    const auto& y_values = ybus.get_values();
    const auto& y_cols   = ybus.get_col_indices();
    const auto& y_rows   = ybus.get_row_ptr();

    // Precompute unit-direction vectors: V_unit_dir[i][p] = V[i][p] / |V[i][p]|
    std::vector<ComplexVector3> V_unit_dir(
        static_cast<std::size_t>(num_nodes), ComplexVector3(3, zero));
    for (int i = 0; i < num_nodes; i++) {
        for (int p = 0; p < 3; p++) {
            const double mag_v = std::abs(V[static_cast<std::size_t>(i)][p]);
            if (mag_v > 0.0) {
                V_unit_dir[static_cast<std::size_t>(i)][p] =
                    V[static_cast<std::size_t>(i)][p] / mag_v;
            }
        }
    }

    for (int i = 1; i < num_nodes; i++) {
        const ComplexVector3& V_i_row   = V[static_cast<std::size_t>(i)];
        const ComplexVector3& S_i_row   = S_calc[static_cast<std::size_t>(i)];
        const ComplexVector3& Vdir_i_row = V_unit_dir[static_cast<std::size_t>(i)];

        for (int ptr = y_rows[i]; ptr < y_rows[i + 1]; ptr++) {
            const int k = y_cols[ptr];
            const ComplexMatrix3x3& Y_block = y_values[ptr];
            const ComplexVector3& V_k_row    = V[static_cast<std::size_t>(k)];
            const ComplexVector3& Vdir_k_row = V_unit_dir[static_cast<std::size_t>(k)];

            for (int p_i = 0; p_i < 3; p_i++) {
                const int row_base = state_index.idx[static_cast<std::size_t>(i)][p_i];
                if (row_base == -1) continue; // phase absent at node i

                const int row_P = row_base;
                const int row_Q = row_base + 1;

                const std::complex<double> V_i    = V_i_row[p_i];
                const std::complex<double> S_i    = S_i_row[p_i];
                const std::complex<double> Vdir_i = Vdir_i_row[p_i];

                for (int p_k = 0; p_k < 3; p_k++) {
                    const int col_base = state_index.idx[static_cast<std::size_t>(k)][p_k];
                    if (col_base == -1) continue; // phase absent at node k (or k is slack)

                    const int col_Theta = col_base;
                    const int col_Vmag  = col_base + 1;

                    std::complex<double> dS_dTheta;
                    std::complex<double> dS_dVmag;

                    if (i == k && p_i == p_k) {
                        // Diagonal element (same node, same phase)
                        const std::complex<double> Y_ii = Y_block(p_i, p_i);
                        dS_dTheta = J_MATH * S_i - J_MATH * V_i * std::conj(Y_ii * V_i);
                        dS_dVmag  = Vdir_i * std::conj(S_i / V_i)
                                  + V_i    * std::conj(Y_ii * Vdir_i);
                    } else {
                        // Off-diagonal (different node or different phase)
                        const std::complex<double> V_k   = V_k_row[p_k];
                        const std::complex<double> Y_ik  = Y_block(p_i, p_k);
                        dS_dTheta = -J_MATH * V_i * std::conj(Y_ik * V_k);
                        dS_dVmag  =           V_i * std::conj(Y_ik * Vdir_k_row[p_k]);
                    }

                    J[static_cast<std::size_t>(row_P)][static_cast<std::size_t>(col_Theta)] = dS_dTheta.real();
                    J[static_cast<std::size_t>(row_Q)][static_cast<std::size_t>(col_Theta)] = dS_dTheta.imag();
                    J[static_cast<std::size_t>(row_P)][static_cast<std::size_t>(col_Vmag)]  = dS_dVmag.real();
                    J[static_cast<std::size_t>(row_Q)][static_cast<std::size_t>(col_Vmag)]  = dS_dVmag.imag();
                }
            }
        }
    }

    std::cout << "   -> Jacobian successfully built with size "
              << dimension << "x" << dimension << "!\n";
    return J;
}

