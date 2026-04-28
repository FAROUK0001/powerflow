#pragma once
#include "core/linalg/vector.hpp"
#include <vector>
#include <stdexcept>
#include <algorithm> // For std::sort

template <typename T>
class MatrixSparseCSR {
private:
    struct Triplet {
        int row;
        int col;
        T value;
    };
    std::vector<Triplet> temp_triplets;

    int num_rows;
    int num_cols;
    std::vector<T> values;
    std::vector<int> col_indices;
    std::vector<int> row_ptr;

public:
    MatrixSparseCSR(int rows, int cols) : num_rows(rows), num_cols(cols) {
        // row_ptr must always have exactly (rows + 1) elements, starting at 0.
        row_ptr.assign(num_rows + 1, 0);
    }

    void add_value(int row, int col, T val) {
        temp_triplets.push_back({row, col, val});
    }

    // Build (or rebuild) the CSR arrays from the accumulated triplets.
    // Safe to call multiple times: previous CSR data is cleared first.
    void build_csr() {
        if (temp_triplets.empty()) return;

        // Clear any previously built data before rebuilding.
        values.clear();
        col_indices.clear();
        row_ptr.assign(num_rows + 1, 0);

        // STEP 1: Sort the triplets left-to-right, top-to-bottom.
        std::sort(temp_triplets.begin(), temp_triplets.end(),
            [](const Triplet& a, const Triplet& b) {
                if (a.row == b.row) {
                    return a.col < b.col;
                }
                return a.row < b.row;
            }
        );

        // STEP 2: Reserve exact capacity to avoid repeated reallocation.
        const int non_zeros = static_cast<int>(temp_triplets.size());
        values.reserve(static_cast<std::size_t>(non_zeros));
        col_indices.reserve(static_cast<std::size_t>(non_zeros));

        // STEP 3: Fill values/col_indices and count non-zeros per row.
        for (const auto& t : temp_triplets) {
            values.push_back(t.value);
            col_indices.push_back(t.col);
            // Temporarily store the count of non-zeros for each row at row+1.
            ++row_ptr[t.row + 1];
        }

        // STEP 4: Convert per-row counts into cumulative start indices.
        for (int i = 0; i < num_rows; i++) {
            row_ptr[i + 1] += row_ptr[i];
        }

        // STEP 5: Release the temporary triplets to free RAM.
        temp_triplets.clear();
        temp_triplets.shrink_to_fit();
    }

    // CSR matrix-vector multiplication.
    Vector<T> operator*(const Vector<T>& x) const {
        if (static_cast<int>(x.size()) != num_cols) {
            throw std::invalid_argument("Vector size must match Matrix columns!");
        }

        Vector<T> result(num_rows);

        for (int i = 0; i < num_rows; i++) {
            T row_sum = T(0);
            const int row_start = row_ptr[i];
            const int row_end   = row_ptr[i + 1];

            for (int j = row_start; j < row_end; j++) {
                row_sum = row_sum + values[j] * x[col_indices[j]];
            }

            result[i] = row_sum;
        }

        return result;
    }

    const std::vector<T>& get_values()      const { return values; }
    const std::vector<int>& get_col_indices() const { return col_indices; }
    const std::vector<int>& get_row_ptr()    const { return row_ptr; }
};

