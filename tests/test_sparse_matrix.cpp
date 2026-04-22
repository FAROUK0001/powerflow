#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "core/linalg/vector.hpp"
#include "core/linalg/matrix_sparse_csr.hpp"

TEST_CASE("CSR sparse matrix multiplies a real vector", "[linalg][sparse][real]") {
    MatrixSparseCSR<double> A(3, 3);
    Vector<double> x(3);

    x[0] = 10.0;
    x[1] = 20.0;
    x[2] = 30.0;

    A.add_value(1, 2, 2.0);
    A.add_value(2, 2, 3.0);
    A.add_value(0, 0, 5.0);
    A.add_value(1, 1, 8.0);
    A.build_csr();

    const Vector<double> result = A * x;

    REQUIRE(result[0] == Catch::Approx(50.0));
    REQUIRE(result[1] == Catch::Approx(220.0));
    REQUIRE(result[2] == Catch::Approx(90.0));
}
