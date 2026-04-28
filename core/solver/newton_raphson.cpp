#include "core/solver/newton_raphson.hpp"
#include "core/solver/jacobian_builder.hpp"
#include "core/solver/convergence.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

// The slack bus is always node 0 (the substation source bus).
static constexpr int SLACK_BUS_INDEX = 0;

// Pivot values smaller than this threshold indicate a singular Jacobian row.
static constexpr double SINGULARITY_TOLERANCE = 1e-12;

// ---------------------------------------------------------------------------
// Gaussian elimination with partial pivoting.
// Solves A * x = b and returns x.
// A and b are passed by value (they are consumed during elimination).
// ---------------------------------------------------------------------------
static std::vector<double> gauss_elim(
    std::vector<std::vector<double>> A,
    std::vector<double> b)
{
    const int n = static_cast<int>(b.size());

    for (int col = 0; col < n; ++col) {
        // Find pivot row (partial pivoting for numerical stability)
        int pivot = col;
        for (int row = col + 1; row < n; ++row) {
            if (std::abs(A[row][col]) > std::abs(A[pivot][col])) {
                pivot = row;
            }
        }
        std::swap(A[col], A[pivot]);
        std::swap(b[col], b[pivot]);

        if (std::abs(A[col][col]) < SINGULARITY_TOLERANCE) {
            throw std::runtime_error(
                "Singular Jacobian at column " + std::to_string(col) +
                " — check for isolated or all-zero-phase nodes.");
        }

        // Eliminate entries below the pivot
        for (int row = col + 1; row < n; ++row) {
            const double factor = A[row][col] / A[col][col];
            for (int k = col; k < n; ++k) {
                A[row][k] -= factor * A[col][k];
            }
            b[row] -= factor * b[col];
        }
    }

    // Back substitution
    std::vector<double> x(static_cast<std::size_t>(n), 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[static_cast<std::size_t>(i)] = b[static_cast<std::size_t>(i)];
        for (int j = i + 1; j < n; ++j) {
            x[static_cast<std::size_t>(i)] -= A[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
                                            * x[static_cast<std::size_t>(j)];
        }
        x[static_cast<std::size_t>(i)] /= A[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)];
    }
    return x;
}

// ---------------------------------------------------------------------------
// Recompute S_calc from the current voltage vector.
// ---------------------------------------------------------------------------
static void compute_s_calc(
    std::vector<std::vector<std::complex<double>>>& S_calc,
    const std::vector<std::vector<std::complex<double>>>& V,
    const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
    int num_nodes)
{
    const auto& y_values = ybus.get_values();
    const auto& y_cols   = ybus.get_col_indices();
    const auto& y_rows   = ybus.get_row_ptr();

    const std::complex<double> zero(0.0, 0.0);
    for (int i = 0; i < num_nodes; i++) {
        for (int p = 0; p < 3; p++) {
            S_calc[i][p] = zero;
        }
    }

    for (int i = 0; i < num_nodes; i++) {
        const auto& v_i = V[i];
        auto& s_i = S_calc[i];
        const int row_start = y_rows[i];
        const int row_end   = y_rows[i + 1];

        for (int p_i = 0; p_i < 3; p_i++) {
            std::complex<double> sum_YV(0.0, 0.0);
            for (int ptr = row_start; ptr < row_end; ptr++) {
                const int j = y_cols[ptr];
                const ComplexMatrix3x3& Y_block = y_values[ptr];
                const auto& v_j = V[j];
                for (int p_j = 0; p_j < 3; p_j++) {
                    sum_YV += Y_block(p_i, p_j) * v_j[p_j];
                }
            }
            s_i[p_i] = v_i[p_i] * std::conj(sum_YV);
        }
    }
}

// ---------------------------------------------------------------------------
// Compute mismatch for all non-slack nodes (node 0 is the slack bus).
// ---------------------------------------------------------------------------
static void compute_mismatch(
    std::vector<std::vector<std::complex<double>>>& mismatch,
    const std::vector<std::vector<std::complex<double>>>& S_spec,
    const std::vector<std::vector<std::complex<double>>>& S_calc,
    int num_nodes)
{
    for (int i = SLACK_BUS_INDEX + 1; i < num_nodes; i++) { // Skip slack bus
        for (int p = 0; p < 3; p++) {
            mismatch[i][p] = S_spec[i][p] - S_calc[i][p];
        }
    }
}

// ---------------------------------------------------------------------------
// Flatten the complex mismatch into a real right-hand-side vector f.
// Layout mirrors the Jacobian: for each non-slack node i and phase p,
//   f[(i-1)*6 + p*2    ] = ΔP  (real part of mismatch)
//   f[(i-1)*6 + p*2 + 1] = ΔQ  (imag part of mismatch)
// ---------------------------------------------------------------------------
static std::vector<double> flatten_mismatch(
    const std::vector<std::vector<std::complex<double>>>& mismatch,
    int num_nodes)
{
    const int dim = (num_nodes - 1) * 6;
    std::vector<double> f(static_cast<std::size_t>(dim), 0.0);
    for (int i = SLACK_BUS_INDEX + 1; i < num_nodes; i++) {
        const int row_base = (i - 1) * 6;
        for (int p = 0; p < 3; p++) {
            f[static_cast<std::size_t>(row_base + p * 2)]     = mismatch[i][p].real();
            f[static_cast<std::size_t>(row_base + p * 2 + 1)] = mismatch[i][p].imag();
        }
    }
    return f;
}

// ---------------------------------------------------------------------------
// Apply the Newton step Δx to the voltage vector.
// State layout: Δx[(i-1)*6 + p*2] = Δθ, Δx[(i-1)*6 + p*2 + 1] = Δ|V|
// Voltage update: V_new = (|V| + Δ|V|) * e^{j(θ + Δθ)}
// ---------------------------------------------------------------------------
static void update_voltages(
    std::vector<std::vector<std::complex<double>>>& V,
    const std::vector<double>& delta_x,
    int num_nodes)
{
    for (int i = SLACK_BUS_INDEX + 1; i < num_nodes; i++) { // Slack bus is not updated
        const int row_base = (i - 1) * 6;
        for (int p = 0; p < 3; p++) {
            const double d_theta  = delta_x[static_cast<std::size_t>(row_base + p * 2)];
            const double d_vmag   = delta_x[static_cast<std::size_t>(row_base + p * 2 + 1)];
            const double old_mag  = std::abs(V[i][p]);
            const double old_ang  = std::arg(V[i][p]);
            const double new_mag  = old_mag + d_vmag;
            const double new_ang  = old_ang + d_theta;
            V[i][p] = std::polar(new_mag, new_ang);
        }
    }
}

// ---------------------------------------------------------------------------
// Main Newton-Raphson solver
// ---------------------------------------------------------------------------
SolverResult NewtonRaphson::solve(
    const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
    const std::unordered_map<std::string, Load>& spot_loads,
    const std::vector<DistributedLoad>& dist_loads,
    const std::unordered_map<std::string, int>& node_to_index,
    int    max_iter,
    double tolerance)
{
    const int num_nodes = static_cast<int>(node_to_index.size());
    const std::complex<double> zero(0.0, 0.0);

    // ------------------------------------------------------------------
    // 1. Flat-start voltages (balanced 3-phase at rated magnitude)
    // ------------------------------------------------------------------
    const ComplexVector3 zero_vector(3, zero);
    std::vector<ComplexVector3> V(static_cast<std::size_t>(num_nodes), zero_vector);

    // Phase angles: A = 0°, B = -120°, C = +120°
    // The magnitude will be updated by the NR iterations; start at 1 pu
    // using the same value as the original code (24900 V L-N).
    const double v_mag = 24900.0 / std::sqrt(3.0);
    const std::complex<double> vA = std::polar(v_mag,  0.0);
    const std::complex<double> vB = std::polar(v_mag, -120.0 * (M_PI / 180.0));
    const std::complex<double> vC = std::polar(v_mag,  120.0 * (M_PI / 180.0));

    for (int i = 0; i < num_nodes; i++) {
        V[static_cast<std::size_t>(i)][0] = vA;
        V[static_cast<std::size_t>(i)][1] = vB;
        V[static_cast<std::size_t>(i)][2] = vC;
    }

    // ------------------------------------------------------------------
    // 2. Specified power injections S_spec (computed once)
    // ------------------------------------------------------------------
    std::vector<ComplexVector3> S_spec(static_cast<std::size_t>(num_nodes), zero_vector);

    for (const auto& [node_name, load] : spot_loads) {
        const auto node_it = node_to_index.find(node_name);
        if (node_it == node_to_index.end()) continue;
        const int idx = node_it->second;
        for (int phase = 0; phase < 3; phase++) {
            S_spec[static_cast<std::size_t>(idx)][phase] +=
                std::complex<double>(-load.kw[phase] * 1000.0, -load.kvar[phase] * 1000.0);
        }
    }

    for (const auto& d_load : dist_loads) {
        const auto na_it = node_to_index.find(d_load.node_a);
        const auto nb_it = node_to_index.find(d_load.node_b);
        if (na_it == node_to_index.end() || nb_it == node_to_index.end()) continue;
        const int node_a = na_it->second;
        const int node_b = nb_it->second;
        for (int phase = 0; phase < 3; phase++) {
            const double p_half = -0.5 * (d_load.kw[phase]   * 1000.0);
            const double q_half = -0.5 * (d_load.kvar[phase] * 1000.0);
            S_spec[static_cast<std::size_t>(node_a)][phase] += std::complex<double>(p_half, q_half);
            S_spec[static_cast<std::size_t>(node_b)][phase] += std::complex<double>(p_half, q_half);
        }
    }

    // ------------------------------------------------------------------
    // 3. Newton-Raphson iteration loop
    // ------------------------------------------------------------------
    std::vector<ComplexVector3> S_calc(static_cast<std::size_t>(num_nodes), zero_vector);
    std::vector<ComplexVector3> mismatch(static_cast<std::size_t>(num_nodes), zero_vector);

    int    iter = 0;
    double norm = 0.0;

    for (; iter < max_iter; ++iter) {
        // a) Compute S_calc from current voltages
        compute_s_calc(S_calc, V, ybus, num_nodes);

        // b) Compute power mismatch
        compute_mismatch(mismatch, S_spec, S_calc, num_nodes);

        // c) Evaluate convergence
        norm = convergence::mismatch_norm(mismatch, SLACK_BUS_INDEX + 1);
        if (convergence::has_converged(norm, tolerance)) {
            break;
        }

        // d) Build Jacobian
        RealMatrix J = JacobianBuilder::build(ybus, V, S_calc, num_nodes);

        // e) Flatten mismatch to RHS vector and solve J * Δx = f
        std::vector<double> f = flatten_mismatch(mismatch, num_nodes);
        std::vector<double> delta_x = gauss_elim(std::move(J), f);

        // f) Update voltages
        update_voltages(V, delta_x, num_nodes);
    }

    // ------------------------------------------------------------------
    // 4. Final state: recompute S_calc / mismatch and build Jacobian
    //    so that the returned values always reflect the final voltages.
    // ------------------------------------------------------------------
    compute_s_calc(S_calc, V, ybus, num_nodes);
    compute_mismatch(mismatch, S_spec, S_calc, num_nodes);
    norm = convergence::mismatch_norm(mismatch, SLACK_BUS_INDEX + 1);
    RealMatrix J_final = JacobianBuilder::build(ybus, V, S_calc, num_nodes);

    const bool converged = convergence::has_converged(norm, tolerance);

    if (!converged) {
        std::cerr << "NewtonRaphson: did not converge after " << iter
                  << " iteration(s). Final mismatch norm = " << norm << " VA\n";
    }

    return { V, S_spec, S_calc, mismatch, J_final, iter, norm, converged };
}

