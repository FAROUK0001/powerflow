#pragma once
#include <vector>
#include <stdexcept>
#include <cstddef>

template <typename T>
class Vector {
private:
    std::vector<T> data;

public:
    // Constructors
    explicit Vector(std::size_t size) : data(size, T(0)) {}

    // Convenience overload for int literals (avoids narrowing-conversion warnings
    // at call sites that still use int).
    explicit Vector(int size) : data(static_cast<std::size_t>(size), T(0)) {}

    // Copy / move – rely on the implicitly generated ones from std::vector.
    Vector(const Vector&)            = default;
    Vector& operator=(const Vector&) = default;
    Vector(Vector&&)                 = default;
    Vector& operator=(Vector&&)      = default;

    // Size
    [[nodiscard]] std::size_t size() const { return data.size(); }

    // Unchecked element access (fast path, matches original API)
    T& operator[](std::size_t index) { return data[index]; }
    const T& operator[](std::size_t index) const { return data[index]; }

    // Overloads for int indices (backwards compatibility)
    T& operator[](int index) { return data[static_cast<std::size_t>(index)]; }
    const T& operator[](int index) const { return data[static_cast<std::size_t>(index)]; }

    // Bounds-checked access
    T& at(std::size_t index) { return data.at(index); }
    const T& at(std::size_t index) const { return data.at(index); }

    // Iterators
    using iterator       = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    iterator       begin()        { return data.begin(); }
    iterator       end()          { return data.end(); }
    const_iterator begin()  const { return data.begin(); }
    const_iterator end()    const { return data.end(); }
    const_iterator cbegin() const { return data.cbegin(); }
    const_iterator cend()   const { return data.cend(); }

    // Named setter (kept for API parity)
    void set(std::size_t index, const T& value) { data[index] = value; }

    // Scalar multiplication
    Vector<T> operator*(const T& scalar) const {
        Vector<T> result(data.size());
        for (std::size_t i = 0; i < data.size(); ++i) {
            result.data[i] = data[i] * scalar;
        }
        return result;
    }

    // Vector addition
    Vector<T> operator+(const Vector<T>& other) const {
        if (data.size() != other.data.size()) {
            throw std::invalid_argument("Vector sizes must match for addition");
        }
        Vector<T> result(data.size());
        for (std::size_t i = 0; i < data.size(); ++i) {
            result.data[i] = data[i] + other.data[i];
        }
        return result;
    }

    // Vector subtraction
    Vector<T> operator-(const Vector<T>& other) const {
        if (data.size() != other.data.size()) {
            throw std::invalid_argument("Vector sizes must match for subtraction");
        }
        Vector<T> result(data.size());
        for (std::size_t i = 0; i < data.size(); ++i) {
            result.data[i] = data[i] - other.data[i];
        }
        return result;
    }
};

