#include <iostream>
#include <complex>
#include "core/linalg/vector.hpp"
#include "core/linalg/matrix_sparse_csr.hpp"

using Complex = std::complex<double>;

// Helper function to print complex numbers
void print_complex(const std::string& name, const Complex& c) {
    std::cout << name << ": " << c.real() << (c.imag() >= 0 ? " + j" : " - j") << std::abs(c.imag()) << '\n';
}

int main() {
    std::cout << "--- Testing Real 2-Bus Power Grid ---\n\n";

    // 1. The Given Line Data
    Complex Z = Complex(0.0, 0.1);    // Impedance Z = j0.1
    double B = 0.2;                   // Line charging susceptance B = 0.2

    // 2. The Math (C++ complex library does this automatically!)
    Complex y_series = 1.0 / Z;                           // y = 1/Z = -j10.0
    Complex y_shunt  = Complex(0.0, B / 2.0);             // y_sh = j0.1

    std::cout << "Calculated Line Parameters:\n";
    print_complex("Series Admittance (y)", y_series);
    print_complex("Shunt Admittance (y_sh)", y_shunt);
    std::cout << "\n";

    // 3. Build the 2x2 Y-Bus Matrix
    MatrixSparseCSR<Complex> Ybus(2, 2);

    // Diagonal Elements: Y11 and Y22 = y_series + y_shunt
    Ybus.add_value(0, 0, y_series + y_shunt); // Bus 1
    Ybus.add_value(1, 1, y_series + y_shunt); // Bus 2

    // Off-Diagonal Elements: Y12 and Y21 = -y_series
    Ybus.add_value(0, 1, -y_series); // Bus 1 to Bus 2
    Ybus.add_value(1, 0, -y_series); // Bus 2 to Bus 1

    Ybus.build_csr(); // Compress it!

    // 4. Set the Bus Voltages
    Vector<Complex> V(2);
    V[0] = Complex(1.0, 0.0);   // Bus 1 (Slack Bus at 1.0 pu)
    V[1] = Complex(0.95, -0.1); // Bus 2 (Voltage dropped a bit)

    // 5. CALCULATE CURRENT! (I = Y * V)
    std::cout << "Calculating Current Injections (I = Y * V)...\n\n";
    Vector<Complex> I = Ybus * V;

    // 6. Output the Results
    std::cout << "Expected Currents:\n";
    std::cout << "I1: 1 + j(-0.4)\n";
    std::cout << "I2: -0.99 + j0.595\n\n";

    std::cout << "Actual Currents:\n";
    print_complex("I1", I[0]);
    print_complex("I2", I[1]);

    // Check if the math engine got it right!
    // We use a tiny tolerance (1e-6) because computers sometimes have floating point rounding errors.
    if (std::abs(I[0] - Complex(1.0, -0.4)) < 1e-6 &&
        std::abs(I[1] - Complex(-0.99, 0.595)) < 1e-6) {
        std::cout << "\n✅ TEST PASSED! The Power Flow Math Engine is fully functional!\n";
    } else {
        std::cout << "\n❌ TEST FAILED!\n";
    }

    return 0;
}