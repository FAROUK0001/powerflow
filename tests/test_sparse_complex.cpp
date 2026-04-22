#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <complex>
#include "core/linalg/vector.hpp"
#include "core/linalg/matrix_sparse_csr.hpp"

static void require_complex_approx(
    const std::complex<double>& actual,
    const std::complex<double>& expected
) {
    REQUIRE(actual.real() == Catch::Approx(expected.real()));
    REQUIRE(actual.imag() == Catch::Approx(expected.imag()));
}

TEST_CASE("CSR sparse matrix multiplies a complex vector", "[linalg][sparse][complex]") {
    MatrixSparseCSR<std::complex<double>> A(2, 2);
    Vector<std::complex<double>> x(2);

    x[0] = std::complex<double>(1.0, 1.0);  // 1 + j1
    x[1] = std::complex<double>(2.0, -1.0); // 2 - j1

    A.add_value(1, 1, std::complex<double>(4.0, 0.0));
    A.add_value(0, 0, std::complex<double>(1.0, 2.0));
    A.add_value(1, 0, std::complex<double>(0.0, 3.0));

    A.build_csr();

    const Vector<std::complex<double>> result = A * x;

    require_complex_approx(result[0], std::complex<double>(-1.0, 3.0));
    require_complex_approx(result[1], std::complex<double>(5.0, -1.0));
}
