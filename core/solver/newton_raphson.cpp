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
// Detect which phases are physically active at each node.
// A phase p is considered present at node i when the diagonal entry
// Y[i][i](p,p) of the Ybus is non-zero (magnitude > threshold).
// The slack bus always has all phases active regardless.
// ---------------------------------------------------------------------------
static std::vector<std::array<bool, 3>> detect_phases(
    const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
    int num_nodes)
{
    std::vector<std::array<bool, 3>> phase_exists(
        static_cast<std::size_t>(num_nodes),
        std::array<bool, 3>{false, false, false});

    const auto& y_values = ybus.get_values();
    const auto& y_cols   = ybus.get_col_indices();
    const auto& y_rows   = ybus.get_row_ptr();

    for (int i = 0; i < num_nodes; i++) {
        for (int ptr = y_rows[i]; ptr < y_rows[i + 1]; ptr++) {
            if (y_cols[ptr] != i) continue; // only the self-admittance (diagonal) block
            const ComplexMatrix3x3& Y_diag = y_values[ptr];
            for (int p = 0; p < 3; p++) {
                if (std::abs(Y_diag(p, p)) > 1e-12) {
                    phase_exists[static_cast<std::size_t>(i)][p] = true;
                }
            }
        }
    }

    // The slack bus is always treated as fully 3-phase.
    phase_exists[SLACK_BUS_INDEX] = {true, true, true};
    return phase_exists;
}

// ---------------------------------------------------------------------------
// Build the sparse state index that maps (node, phase) -> base offset.
// Only non-slack nodes with existing phases are included.
// Each active (node, phase) pair occupies two consecutive slots:
//   [base]   = Δθ  (angle update)
//   [base+1] = Δ|V| (magnitude update)
// ---------------------------------------------------------------------------
static StateIndex build_state_index(
    const std::vector<std::array<bool, 3>>& phase_exists,
    int num_nodes)
{
    StateIndex si;
    si.idx.assign(
        static_cast<std::size_t>(num_nodes),
        std::array<int, 3>{-1, -1, -1});
    si.dimension = 0;

    for (int i = SLACK_BUS_INDEX + 1; i < num_nodes; i++) {
        for (int p = 0; p < 3; p++) {
            if (phase_exists[static_cast<std::size_t>(i)][p]) {
                si.idx[static_cast<std::size_t>(i)][p] = si.dimension;
                si.dimension += 2; // theta and magnitude
            }
        }
    }
    return si;
}

// ---------------------------------------------------------------------------
// Recompute S_calc from the current voltage vector.
// Only contributions from existing phases are accumulated.
// ---------------------------------------------------------------------------
static void compute_s_calc(
    std::vector<std::vector<std::complex<double>>>& S_calc,
    const std::vector<std::vector<std::complex<double>>>& V,
    const MatrixSparseCSR<ComplexMatrix3x3>& ybus,
    const std::vector<std::array<bool, 3>>& phase_exists,
    int num_nodes)
{
    const auto& y_values = ybus.get_values();
    const auto& y_cols   = ybus.get_col_indices();
    const auto& y_rows   = ybus.get_row_ptr();

    const std::complex<double> zero(0.0, 0.0);
    for (int i = 0; i < num_nodes; i++) {
        for (int p = 0; p < 3; p++) {
            S_calc[static_cast<std::size_t>(i)][p] = zero;
        }
    }

    for (int i = 0; i < num_nodes; i++) {
        const auto& v_i = V[static_cast<std::size_t>(i)];
        auto& s_i = S_calc[static_cast<std::size_t>(i)];
        const int row_start = y_rows[i];
        const int row_end   = y_rows[i + 1];

        for (int p_i = 0; p_i < 3; p_i++) {
            if (!phase_exists[static_cast<std::size_t>(i)][p_i]) continue;

            std::complex<double> sum_YV(0.0, 0.0);
            for (int ptr = row_start; ptr < row_end; ptr++) {
                const int j = y_cols[ptr];
                const ComplexMatrix3x3& Y_block = y_values[ptr];
                const auto& v_j = V[static_cast<std::size_t>(j)];
                for (int p_j = 0; p_j < 3; p_j++) {
                    if (!phase_exists[static_cast<std::size_t>(j)][p_j]) continue;
                    sum_YV += Y_block(p_i, p_j) * v_j[p_j];
                }
            }
            s_i[p_i] = v_i[p_i] * std::conj(sum_YV);
        }
    }
}

// ---------------------------------------------------------------------------
// Compute mismatch for all non-slack nodes.
// Missing phases are left at zero.
// ---------------------------------------------------------------------------
static void compute_mismatch(
    std::vector<std::vector<std::complex<double>>>& mismatch,
    const std::vector<std::vector<std::complex<double>>>& S_spec,
    const std::vector<std::vector<std::complex<double>>>& S_calc,
    const std::vector<std::array<bool, 3>>& phase_exists,
    int num_nodes)
{
    for (int i = SLACK_BUS_INDEX + 1; i < num_nodes; i++) {
        for (int p = 0; p < 3; p++) {
            if (!phase_exists[static_cast<std::size_t>(i)][p]) {
                mismatch[static_cast<std::size_t>(i)][p] = std::complex<double>(0.0, 0.0);
                continue;
            }
            mismatch[static_cast<std::size_t>(i)][p] =
                S_spec[static_cast<std::size_t>(i)][p] - S_calc[static_cast<std::size_t>(i)][p];
        }
    }
}

// ---------------------------------------------------------------------------
// Flatten the complex mismatch into a real right-hand-side vector f.
// Layout follows StateIndex: for each active (node, phase),
//   f[base]   = ΔP  (real part of mismatch)
//   f[base+1] = ΔQ  (imag part of mismatch)
// ---------------------------------------------------------------------------
static std::vector<double> flatten_mismatch(
    const std::vector<std::vector<std::complex<double>>>& mismatch,
    const StateIndex& state_index,
    int num_nodes)
{
    std::vector<double> f(static_cast<std::size_t>(state_index.dimension), 0.0);
    for (int i = SLACK_BUS_INDEX + 1; i < num_nodes; i++) {
        for (int p = 0; p < 3; p++) {
            const int base = state_index.idx[static_cast<std::size_t>(i)][p];
            if (base == -1) continue;
            f[static_cast<std::size_t>(base)]     = mismatch[static_cast<std::size_t>(i)][p].real();
            f[static_cast<std::size_t>(base + 1)] = mismatch[static_cast<std::size_t>(i)][p].imag();
        }
    }
    return f;
}

// ---------------------------------------------------------------------------
// Apply the Newton step Δx to the voltage vector.
// Uses StateIndex to map (node, phase) -> (Δθ, Δ|V|).
// Voltage update: V_new = (|V| + Δ|V|) * e^{j(θ + Δθ)}
// ---------------------------------------------------------------------------
static void update_voltages(
    std::vector<std::vector<std::complex<double>>>& V,
    const std::vector<double>& delta_x,
    const StateIndex& state_index,
    int num_nodes)
{
    for (int i = SLACK_BUS_INDEX + 1; i < num_nodes; i++) {
        for (int p = 0; p < 3; p++) {
            const int base = state_index.idx[static_cast<std::size_t>(i)][p];
            if (base == -1) continue;
            const double d_theta = delta_x[static_cast<std::size_t>(base)];
            const double d_vmag  = delta_x[static_cast<std::size_t>(base + 1)];
            const double old_mag = std::abs(V[static_cast<std::size_t>(i)][p]);
            const double old_ang = std::arg(V[static_cast<std::size_t>(i)][p]);
            V[static_cast<std::size_t>(i)][p] = std::polar(old_mag + d_vmag, old_ang + d_theta);
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
    // 1. Detect active phases and build sparse state index
    // ------------------------------------------------------------------
    const auto phase_exists = detect_phases(ybus, num_nodes);
    const StateIndex state_index = build_state_index(phase_exists, num_nodes);

    std::cout << "   -> Sparse state dimension: " << state_index.dimension
              << " variables (vs fixed " << (num_nodes - 1) * 6 << ")\n";

    // ------------------------------------------------------------------
    // 2. Flat-start voltages
    //    Existing phases: balanced 3-phase at rated magnitude (24900/√3 V L-N).
    //    Absent phases:   zero (they are not part of the state vector).
    // ------------------------------------------------------------------
    const ComplexVector3 zero_vector(3, zero);
    std::vector<ComplexVector3> V(static_cast<std::size_t>(num_nodes), zero_vector);

    const double v_mag = 24900.0 / std::sqrt(3.0);
    const std::complex<double> vA = std::polar(v_mag,  0.0);
    const std::complex<double> vB = std::polar(v_mag, -120.0 * (M_PI / 180.0));
    const std::complex<double> vC = std::polar(v_mag,  120.0 * (M_PI / 180.0));
    const std::complex<double> v_init[3] = {vA, vB, vC};

    for (int i = 0; i < num_nodes; i++) {
        for (int p = 0; p < 3; p++) {
            V[static_cast<std::size_t>(i)][p] =
                phase_exists[static_cast<std::size_t>(i)][p] ? v_init[p] : zero;
        }
    }

    // ------------------------------------------------------------------
    // 3. Specified power injections S_spec (computed once)
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
    // 4. Newton-Raphson iteration loop
    // ------------------------------------------------------------------
    std::vector<ComplexVector3> S_calc(static_cast<std::size_t>(num_nodes), zero_vector);
    std::vector<ComplexVector3> mismatch(static_cast<std::size_t>(num_nodes), zero_vector);

    int    iter = 0;
    double norm = 0.0;

    for (; iter < max_iter; ++iter) {
        // a) Compute S_calc from current voltages
        compute_s_calc(S_calc, V, ybus, phase_exists, num_nodes);

        // b) Compute power mismatch
        compute_mismatch(mismatch, S_spec, S_calc, phase_exists, num_nodes);

        // c) Evaluate convergence
        norm = convergence::mismatch_norm(mismatch, SLACK_BUS_INDEX + 1);
        if (convergence::has_converged(norm, tolerance)) {
            break;
        }

        // d) Build Jacobian
        RealMatrix J = JacobianBuilder::build(ybus, V, S_calc, state_index);

        // e) Flatten mismatch to RHS vector and solve J * Δx = f
        std::vector<double> f = flatten_mismatch(mismatch, state_index, num_nodes);
        std::vector<double> delta_x = gauss_elim(std::move(J), f);

        // f) Update voltages
        update_voltages(V, delta_x, state_index, num_nodes);
    }

    // ------------------------------------------------------------------
    // 5. Final state: recompute S_calc / mismatch and build Jacobian
    //    so that the returned values always reflect the final voltages.
    // ------------------------------------------------------------------
    compute_s_calc(S_calc, V, ybus, phase_exists, num_nodes);
    compute_mismatch(mismatch, S_spec, S_calc, phase_exists, num_nodes);
    norm = convergence::mismatch_norm(mismatch, SLACK_BUS_INDEX + 1);
    RealMatrix J_final = JacobianBuilder::build(ybus, V, S_calc, state_index);

    const bool converged = convergence::has_converged(norm, tolerance);

    if (!converged) {
        std::cerr << "NewtonRaphson: did not converge after " << iter
                  << " iteration(s). Final mismatch norm = " << norm << " VA\n";
    }

    return { V, S_spec, S_calc, mismatch, J_final, iter, norm, converged };
}

