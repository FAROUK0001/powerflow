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

    for (int i = 1; i < num_nodes; i++) {
        for (int ptr = y_rows[i]; ptr < y_rows[i + 1]; ptr++) {

            const int k = y_cols[ptr];
            if (k == 0) continue;

            const ComplexMatrix3x3& Y_block = y_values[ptr];

            for (int p_i = 0; p_i < 3; p_i++) {
                for (int p_k = 0; p_k < 3; p_k++) {

                    std::complex<double> dS_dTheta;
                    std::complex<double> dS_dVmag; // <-- ADDED THE MAGNITUDE VARIABLE

                    if (i == k && p_i == p_k) {
                        // DIAGONAL
                        const std::complex<double> V_i = V[i][p_i];
                        const std::complex<double> Y_ii = Y_block(p_i, p_i);
                        const std::complex<double> S_i = S_calc[i][p_i];

                        // 1. Angle Derivative
                        dS_dTheta = J_MATH * S_i - J_MATH * V_i * std::conj(Y_ii * V_i);

                        // 2. Magnitude Derivative
                        const double mag_V = std::abs(V_i);
                        const std::complex<double> V_dir = V_i / mag_V;
                        dS_dVmag = V_dir * std::conj(S_i / V_i) + V_i * std::conj(Y_ii * V_dir);
                    }
                    else {
                        // OFF-DIAGONAL
                        const std::complex<double> V_i = V[i][p_i];
                        const std::complex<double> V_k = V[k][p_k];
                        const std::complex<double> Y_ik = Y_block(p_i, p_k);

                        // 1. Angle Derivative
                        dS_dTheta = -J_MATH * V_i * std::conj(Y_ik * V_k);

                        // 2. Magnitude Derivative
                        const double mag_V_k = std::abs(V_k);
                        const std::complex<double> V_k_dir = V_k / mag_V_k;
                        dS_dVmag = V_i * std::conj(Y_ik * V_k_dir);
                    }

                    // Separate the complex derivative into Real (dP) and Imaginary (dQ)
                    const double dP_dTheta = dS_dTheta.real();
                    const double dQ_dTheta = dS_dTheta.imag();

                    const double dP_dVmag = dS_dVmag.real();
                    const double dQ_dVmag = dS_dVmag.imag();

                    // Calculate matrix indices
                    const int row_P = (i - 1) * 6 + (p_i * 2);
                    const int row_Q = row_P + 1;

                    const int col_Theta = (k - 1) * 6 + (p_k * 2);
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
