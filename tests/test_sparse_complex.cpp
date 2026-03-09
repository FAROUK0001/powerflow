#include <iostream>
#include <complex>
#include "core/linalg/vector.hpp"
#include "core/linalg/matrix_sparse_csr.hpp"

// A quick helper function to print complex numbers nicely
void print_complex(const std::complex<double>& c) {
    std::cout << c.real() << (c.imag() >= 0 ? " + j" : " - j") << std::abs(c.imag());
}

int main() {
    std::cout << "--- Testing CSR Sparse Matrix with Complex Numbers ---\n\n";

    // 1. Create a 2x2 Matrix and a Vector of size 2 using std::complex
    MatrixSparseCSR<std::complex<double>> A(2, 2);
    Vector<std::complex<double>> x(2);

    // 2. Set up our Complex Vector x
    x[0] = std::complex<double>(1.0, 1.0);  // 1 + j1
    x[1] = std::complex<double>(2.0, -1.0); // 2 - j1

    // 3. Add values to the matrix OUT OF ORDER
    A.add_value(1, 1, std::complex<double>(4.0, 0.0)); // Row 1, Col 1 -> 4 + j0
    A.add_value(0, 0, std::complex<double>(1.0, 2.0)); // Row 0, Col 0 -> 1 + j2
    A.add_value(1, 0, std::complex<double>(0.0, 3.0)); // Row 1, Col 0 -> 0 + j3

    // 4. Compress the matrix!
    std::cout << "Building Complex CSR Arrays...\n";
    A.build_csr();

    // 5. Do the math using your brilliant loop!
    std::cout << "Multiplying A * x...\n\n";
    Vector<std::complex<double>> result = A * x;

    // 6. Print the results
    std::cout << "Expected Result:\n";
    std::cout << "[ -1 + j3 ]\n";
    std::cout << "[  5 - j1 ]\n\n";

    std::cout << "Actual Result:\n[ ";
    print_complex(result[0]);
    std::cout << " ]\n[  ";
    print_complex(result[1]);
    std::cout << " ]\n";

    // Verify it matches exactly
    if (result[0] == std::complex<double>(-1.0, 3.0) &&
        result[1] == std::complex<double>(5.0, -1.0)) {
        std::cout << "\n✅ TEST PASSED! Your C++ templates handle complex math perfectly!\n";
        } else {
            std::cout << "\n❌ TEST FAILED!\n";
        }

    return 0;
}