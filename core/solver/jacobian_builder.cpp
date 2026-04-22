#include "core/solver/jacobian_builder.hpp"
#include <iostream>
#include <cmath>

RealMatrix JacobianBuilder::build(
    const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
    const std::vector<ComplexVector3>& V,
    const std::vector<ComplexVector3>& S_calc,
    int num_nodes
) {
    std::cout << "   -> Assembling the Full Jacobian Matrix...\n";

    // 198 x 198 Matrix
    const int dimension = (num_nodes - 1) * 6;
    RealMatrix J(dimension, std::vector<double>(dimension, 0.0));

    const std::complex<double> J_MATH(0.0, 1.0);

    const auto& y_values = ybus.get_values();
    const auto& y_cols   = ybus.get_col_indices();
    const auto& y_rows   = ybus.get_row_ptr();
    const std::complex<double> zero(0.0, 0.0);

    std::vector<ComplexVector3> V_unit_dir(num_nodes, ComplexVector3(3, zero));
    for (int i = 0; i < num_nodes; i++) {
        for (int p = 0; p < 3; p++) {
            const std::complex<double> v = V[i][p];
            const double mag_v = std::abs(v);
            if (mag_v > 0.0) {
                V_unit_dir[i][p] = v / mag_v;
            }
        }
    }

    for (int i = 1; i < num_nodes; i++) {
        const int row_base = (i - 1) * 6;
        const ComplexVector3& V_i_row = V[i];
        const ComplexVector3& S_i_row = S_calc[i];

        for (int ptr = y_rows[i]; ptr < y_rows[i + 1]; ptr++) {
            const int k = y_cols[ptr];
            if (k == 0) continue;

            const ComplexMatrix3x3& Y_block = y_values[ptr];
            const int col_base = (k - 1) * 6;
            const ComplexVector3& V_k_row = V[k];
            const ComplexVector3& V_k_dir_row = V_unit_dir[k];

            for (int p_i = 0; p_i < 3; p_i++) {
                const std::complex<double> V_i = V_i_row[p_i];
                const std::complex<double> S_i = S_i_row[p_i];
                const std::complex<double> V_i_dir = V_unit_dir[i][p_i];

                for (int p_k = 0; p_k < 3; p_k++) {
                    std::complex<double> dS_dTheta;
                    std::complex<double> dS_dVmag; // <-- ADDED THE MAGNITUDE VARIABLE

                    if (i == k && p_i == p_k) {
                        // DIAGONAL
                        const std::complex<double> Y_ii = Y_block(p_i, p_i);

                        // 1. Angle Derivative
                        dS_dTheta = J_MATH * S_i - J_MATH * V_i * std::conj(Y_ii * V_i);

                        // 2. Magnitude Derivative
                        dS_dVmag = V_i_dir * std::conj(S_i / V_i) + V_i * std::conj(Y_ii * V_i_dir);
                    }
                    else {
                        // OFF-DIAGONAL
                        const std::complex<double> V_k = V_k_row[p_k];
                        const std::complex<double> Y_ik = Y_block(p_i, p_k);

                        // 1. Angle Derivative
                        dS_dTheta = -J_MATH * V_i * std::conj(Y_ik * V_k);

                        // 2. Magnitude Derivative
                        dS_dVmag = V_i * std::conj(Y_ik * V_k_dir_row[p_k]);
                    }

                    // Separate the complex derivative into Real (dP) and Imaginary (dQ)
                    const double dP_dTheta = dS_dTheta.real();
                    const double dQ_dTheta = dS_dTheta.imag();

                    const double dP_dVmag = dS_dVmag.real();
                    const double dQ_dVmag = dS_dVmag.imag();

                    // Calculate matrix indices
                    const int row_P = row_base + (p_i * 2);
                    const int row_Q = row_P + 1;

                    const int col_Theta = col_base + (p_k * 2);
                    const int col_Vmag  = col_Theta + 1; // <-- MAGNITUDES GO IN ODD COLUMNS

                    // Save Angle Derivatives (Knob 1)
                    J[row_P][col_Theta] = dP_dTheta;
                    J[row_Q][col_Theta] = dQ_dTheta;

                    // Save Magnitude Derivatives (Knob 2)
                    J[row_P][col_Vmag] = dP_dVmag;
                    J[row_Q][col_Vmag] = dQ_dVmag;
                }
            }
        }
    }

    std::cout << "   -> Jacobian successfully built with size " << dimension << "x" << dimension << "!\n";
    return J;
}
