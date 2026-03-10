#pragma once
#include <string>

struct Regulator {
    std::string id;
    std::string from_node;
    std::string to_node;
    int phase;          // 1, 2, or 3

    double v_hold;      // Setpoint Voltage
    double r_volt;      // Line Drop Compensator R
    double x_volt;      // Line Drop Compensator X
    double pt_ratio;    // Potential Transformer Ratio
    double ct_rate;     // Current Transformer Rating
    double bandwidth;   // Voltage Deadband
};