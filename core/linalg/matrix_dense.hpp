#pragma once
#include <vector>
#include <stdexcept>
#include "core/linalg/vector.hpp"
#include <iostream>

template <typename T>
class MatrixDense {
private:
    int num_rows;
    int num_cols;
    std::vector<T> data;

public:
    // 1. DEFAULT CONSTRUCTOR (Required by std::unordered_map!)
    MatrixDense() : num_rows(0), num_cols(0) {}

    // 2. Existing Constructor: Create an empty matrix of size rows x cols
    MatrixDense(int rows, int cols) : num_rows(rows), num_cols(cols) {
        data.assign(rows * cols, T(0));
    }

    int rows() const { return num_rows; }
    int cols() const { return num_cols; }

    // Access elements using A(row, col)
    T& operator()(int row, int col) {
        if (row >= num_rows || col >= num_cols) {
            throw std::out_of_range("Matrix Index out of bounds");
        }
        return data[row * num_cols + col];
    }

    // Read elements using A(row, col) (Const version)
    const T& operator()(int row, int col) const {
        if (row >= num_rows || col >= num_cols) {
            throw std::out_of_range("Matrix Index out of bounds");
        }
        return data[row * num_cols + col];
    }

    // --- MATH OPERATIONS ---

    // 1. Matrix Addition: A + B
    MatrixDense<T> operator+(const MatrixDense<T>& other) const {
        if (num_rows != other.num_rows || num_cols != other.num_cols) {
            throw std::invalid_argument("Matrix sizes must match for addition");
        }
        MatrixDense<T> result(num_rows, num_cols);
        for (int i = 0; i < data.size(); i++) {
            result.data[i] = data[i] + other.data[i];
        }
        return result;
    }

    // 2. Matrix * Vector Multiplication (Crucial for Power Flow!)
    Vector<T> operator*(const Vector<T>& vec) const {
        if (num_cols != vec.size()) {
            throw std::invalid_argument("Vector size must match Matrix columns");
        }
        Vector<T> result(num_rows);
        for (int i = 0; i < num_rows; i++) {
            T sum = T(0);
            for (int j = 0; j < num_cols; j++) {
                sum = sum + (*this)(i, j) * vec[j];
            }
            result[i] = sum;
        }
        return result;
    }

    // 3. Matrix * Matrix Multiplication (For chaining 3-phase operations)
    MatrixDense<T> operator*(const MatrixDense<T>& other) const {
        if (num_cols != other.num_rows) {
            throw std::invalid_argument("Matrix A cols must match Matrix B rows");
        }
        MatrixDense<T> result(num_rows, other.num_cols);
        for (int i = 0; i < num_rows; i++) {
            for (int j = 0; j < other.num_cols; j++) {
                T sum = T(0);
                for (int k = 0; k < num_cols; k++) {
                    sum = sum + (*this)(i, k) * other(k, j);
                }
                result(i, j) = sum;
            }
        }
        return result;
    }
       // --- SMART MATRIX INVERSE (Handles 1-phase, 2-phase, and 3-phase lines) ---
    MatrixDense<T> inverse() const {
        if (num_rows != 3 || num_cols != 3) {
            throw std::runtime_error("Currently, inverse() is only optimized for 3x3 phase matrices!");
        }

        MatrixDense<T> result(3, 3);

        // 1. Check which phases actually exist (Diagonal is not zero)
        bool pA = std::abs((*this)(0,0)) > 1e-9;
        bool pB = std::abs((*this)(1,1)) > 1e-9;
        bool pC = std::abs((*this)(2,2)) > 1e-9;

        int active_phases = pA + pB + pC;

        if (active_phases == 3) {
            // --- STANDARD 3x3 INVERSE ---
            T a = (*this)(0,0), b = (*this)(0,1), c = (*this)(0,2);
            T d = (*this)(1,0), e = (*this)(1,1), f = (*this)(1,2);
            T g = (*this)(2,0), h = (*this)(2,1), i = (*this)(2,2);

            T det = a * (e*i - f*h) - b * (d*i - f*g) + c * (d*h - e*g);
            if (std::abs(det) < 1e-12) throw std::runtime_error("3x3 Matrix is singular!");

            T inv_det = T(1.0) / det;
            result(0,0) =  (e*i - f*h) * inv_det; result(0,1) = -(b*i - c*h) * inv_det; result(0,2) =  (b*f - c*e) * inv_det;
            result(1,0) = -(d*i - f*g) * inv_det; result(1,1) =  (a*i - c*g) * inv_det; result(1,2) = -(a*f - c*d) * inv_det;
            result(2,0) =  (d*h - e*g) * inv_det; result(2,1) = -(a*h - b*g) * inv_det; result(2,2) =  (a*e - b*d) * inv_det;

        } else if (active_phases == 2) {
            // --- 2x2 SUB-MATRIX INVERSE ---
            int r1 = -1, r2 = -1;
            if (pA && pB) { r1 = 0; r2 = 1; }
            else if (pA && pC) { r1 = 0; r2 = 2; }
            else if (pB && pC) { r1 = 1; r2 = 2; }

            T a = (*this)(r1,r1), b = (*this)(r1,r2);
            T c = (*this)(r2,r1), d = (*this)(r2,r2);

            T det = (a * d) - (b * c);
            if (std::abs(det) < 1e-12) throw std::runtime_error("2x2 Sub-Matrix is singular!");

            T inv_det = T(1.0) / det;
            result(r1,r1) =  d * inv_det;
            result(r1,r2) = -b * inv_det;
            result(r2,r1) = -c * inv_det;
            result(r2,r2) =  a * inv_det;

        } else if (active_phases == 1) {
            // --- 1-PHASE INVERSE ---
            if (pA) result(0,0) = T(1.0) / (*this)(0,0);
            if (pB) result(1,1) = T(1.0) / (*this)(1,1);
            if (pC) result(2,2) = T(1.0) / (*this)(2,2);
        }

        return result;
    }
    friend std::ostream& operator<<(std::ostream& os, const MatrixDense<T>& mat) {
        for (int i = 0; i < mat.num_rows; i++) {
            os << "[ ";
            for (int j = 0; j < mat.num_cols; j++) {
                os << mat(i, j) << "  ";
            }
            os << "]\n";
        }
        return os;
    }
};