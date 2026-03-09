#include <iostream>
#include "core/linalg/vector.hpp"
#include "core/linalg/matrix_sparse_csr.hpp"

int main() {
    std::cout << "--- Testing CSR Sparse Matrix ---\n\n";

    // 1. Create a 3x3 Matrix and a Vector of size 3
    MatrixSparseCSR<double> A(3, 3);
    Vector<double> x(3);

    // 2. Set up our Vector x = [10, 20, 30]
    x[0] = 10.0;
    x[1] = 20.0;
    x[2] = 30.0;

    // 3. Add values to the matrix OUT OF ORDER to test the sorting!
    // The True Matrix looks like this:
    // [ 5.0,  0.0,  0.0 ]
    // [ 0.0,  8.0,  2.0 ]
    // [ 0.0,  0.0,  3.0 ]

    A.add_value(1, 2, 2.0); // Row 1, Col 2
    A.add_value(2, 2, 3.0); // Row 2, Col 2
    A.add_value(0, 0, 5.0); // Row 0, Col 0
    A.add_value(1, 1, 8.0); // Row 1, Col 1

    // 4. Compress the matrix!
    // This will sort them into (0,0), (1,1), (1,2), (2,2) and build the arrays.
    std::cout << "Building CSR Arrays...\n";
    A.build_csr();

    // 5. Do the math using your brilliant loop!
    std::cout << "Multiplying A * x...\n\n";
    Vector<double> result = A * x;

    // 6. Print the results
    std::cout << "Expected Result: [ 50, 220, 90 ]\n";
    std::cout << "Actual Result:   [ "
              << result[0] << ", "
              << result[1] << ", "
              << result[2] << " ]\n";

    if (result[0] == 50.0 && result[1] == 220.0 && result[2] == 90.0) {
        std::cout << "\n✅ TEST PASSED! Your math engine works perfectly!\n";
    } else {
        std::cout << "\n❌ TEST FAILED!\n";
    }

    return 0;
}