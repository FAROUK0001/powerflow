#pragma once
#include <vector>
#include <stdexcept>

template <typename T>
class Vector {
private:
    std::vector<T> data;

public:
    // Constructor
    explicit Vector(int size) {
        data.resize(size, T(0));
    }

    [[nodiscard]] int size() const {
        return data.size();
    }

    // 🌟 THE PRO UPGRADE: Overloading the [] brackets!
    // This allows you to do: my_vector[i] = 5;
    T& operator[](int index) {
        return data[index];
    }

    // Read-only version of the [] brackets
    const T& operator[](int index) const {
        return data[index];
    }
    void set(int index,T value)
    {
        data[index] = value;
    }

    // --- YOUR PERFECTED SCALAR MULTIPLICATION ---
    Vector<T> operator*(const T& scalar) const {
        // 1. Give the new vector a size!
        Vector<T> result(data.size());

        // 2. Your exact loop
        for (int i = 0; i < data.size(); i++) {
            // 3. Now the [] brackets work perfectly!
            result[i] = data[i] * scalar;
        }

        return result;
    }
    // --- YOUR TURN: VECTOR ADDITION ---
    // We want to be able to do: Vector<T> v3 = v1 + v2;

    Vector<T> operator+(const Vector<T>& other) const {
        // Safety check: Make sure both vectors are the same size!
        if (this->size() != other.size()) {
            throw std::invalid_argument("Vectors must be the same size to add!");
        }
        Vector<T> result(this->size());
        // Step 1: Create a result vector of the correct size

        // Step 2: Write a for-loop
        for (int i = 0; i < this->size(); i++)
        {
            result[i]=data[i]+other[i];
            // Step 3: Add 'data[i]' and 'other.data[i]' and put it in 'result[i]'

        }

        // Step 4: Return the result
        return result;
    }
    Vector<T> operator-(const Vector<T>& other) const {
        // Safety check: Make sure both vectors are the same size!
        if (this->size() != other.size()) {
            throw std::invalid_argument("Vectors must be the same size to add!");
        }
        Vector<T> result(this->size());
        // Step 1: Create a result vector of the correct size

        // Step 2: Write a for-loop
        for (int i = 0; i < this->size(); i++)
        {
            result[i]=data[i]-other[i];
            // Step 3:  subtract! 'data[i]' and 'other.data[i]' and put it in 'result[i]'

        }

        // Step 4: Return the result
        return result;
    }
};