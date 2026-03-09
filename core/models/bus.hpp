#pragma once

// An enum is just a custom list of names we can use instead of typing numbers!
enum class BusType {
    SLACK, // The reference bus (V=1.0, Angle=0)
    PQ,    // Load bus (Known P and Q)
    PV     // Generator bus (Known P and V)
};

struct Bus {
    int id;             // e.g., 800, 802
    BusType type;

    // Voltage
    double v_mag;       // Voltage Magnitude (pu)
    double v_angle;     // Voltage Angle (radians)

    // Power Demand (Load)
    double p_load;      // Real Power Demand
    double q_load;      // Reactive Power Demand

    // Power Generation
    double p_gen;       // Real Power Generation
    double q_gen;       // Reactive Power Generation
};