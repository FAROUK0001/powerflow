#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <complex>
#include "core/linalg/vector.hpp"
#include "core/linalg/matrix_sparse_csr.hpp"

using Complex = std::complex<double>;

static void require_complex_approx(
    const Complex& actual,
    const Complex& expected
) {
    REQUIRE(actual.real() == Catch::Approx(expected.real()));
    REQUIRE(actual.imag() == Catch::Approx(expected.imag()));
}

TEST_CASE("Two-bus current injection matches expected values", "[grid][sparse][complex]") {
    const Complex Z(0.0, 0.1);
    const double B = 0.2;

    const Complex y_series = 1.0 / Z;
    const Complex y_shunt(0.0, B / 2.0);

    MatrixSparseCSR<Complex> Ybus(2, 2);

    Ybus.add_value(0, 0, y_series + y_shunt);
    Ybus.add_value(1, 1, y_series + y_shunt);
    Ybus.add_value(0, 1, -y_series);
    Ybus.add_value(1, 0, -y_series);

    Ybus.build_csr();

    Vector<Complex> V(2);
    V[0] = Complex(1.0, 0.0);
    V[1] = Complex(0.95, -0.1);
    const Vector<Complex> I = Ybus * V;

    require_complex_approx(I[0], Complex(1.0, -0.4));
    require_complex_approx(I[1], Complex(-0.99, 0.595));
}
