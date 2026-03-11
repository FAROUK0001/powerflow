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

    // 🌟 THE MAGIC CSR BUILDER 🌟
    void build_csr() {
        if (temp_triplets.empty()) return; // Nothing to do!

        // STEP 1: Sort the Triplets left-to-right, top-to-bottom.
        std::sort(temp_triplets.begin(), temp_triplets.end(),
            [](const Triplet& a, const Triplet& b) {
                if (a.row == b.row) {
                    return a.col < b.col; // If same row, sort by column
                }
                return a.row < b.row;     // Otherwise, sort by row
            }
        );

        // STEP 2: Allocate exactly enough memory for our final arrays.
        // This is a massive optimization to prevent C++ from constantly resizing arrays.
        int non_zeros = temp_triplets.size();
        values.reserve(non_zeros);
        col_indices.reserve(non_zeros);

        // STEP 3: Fill 'values' and 'col_indices', and count how many items are in each row.
        for (const auto& t : temp_triplets) {
            values.push_back(t.value);
            col_indices.push_back(t.col);

            // This is the trick to building row_ptr!
            // We temporarily store the COUNT of non-zeros for each row.
            // (Notice we use row + 1. You'll see why in Step 4).
            ++row_ptr[t.row + 1];
        }

        // STEP 4: Convert the counts into actual starting indices (Cumulative Sum)
        // Example: If Row 0 has 2 items, Row 1 MUST start at index 2.
        for (int i = 0; i < num_rows; i++) {
            row_ptr[i + 1] += row_ptr[i];
        }

        // STEP 5: Delete the temporary triplets to free up RAM!
        temp_triplets.clear();
        temp_triplets.shrink_to_fit();
    }

    // --- YOUR PERFECTED CSR MATRIX MULTIPLICATION ---
    Vector<T> operator*(const Vector<T>& x) const {
        if (x.size() != num_cols) {
            throw std::invalid_argument("Vector size must match Matrix columns!");
        }

        Vector<T> result(num_rows);

        for (int i = 0; i < num_rows; i++) {
            T row_sum = T(0);

            int row_start = row_ptr[i];
            int row_end   = row_ptr[i + 1];

            // Your brilliant inner loop!
            for (int j = row_start; j < row_end; j++) {
                row_sum = row_sum + values[j] * x[col_indices[j]];
            }

            result[i] = row_sum;
        }

        return result;
    }
    const std::vector<T>& get_values() const { return values; }
    const std::vector<int>& get_col_indices() const { return col_indices; }
    const std::vector<int>& get_row_ptr() const { return row_ptr; }
};
