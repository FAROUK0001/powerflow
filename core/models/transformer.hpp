#pragma once
#include <string>

struct Transformer {
    std::string name;
    double kva;
    double kv_high;
    double kv_low;
    double r_percent;
    double x_percent;
};