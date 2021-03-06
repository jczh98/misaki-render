#include <aspirin/utils.h>
#include <iomanip>
#include <sstream>

namespace aspirin {

// https://github.com/mitsuba-renderer/mitsuba2/blob/master/src/libcore/util.cpp
std::string time_string(float value, bool precise) {
    struct Order {
        float factor;
        const char *suffix;
    };
    const Order orders[] = { { 0, "ms" },      { 1000, "s" }, { 60, "m" },
                             { 60, "h" },      { 24, "d" },   { 7, "w" },
                             { 52.1429f, "y" } };
    if (std::isnan(value)) {
        return "nan";
    } else if (std::isinf(value)) {
        return "inf";
    } else if (value < 0) {
        return "-" + time_string(-value, precise);
    }
    int i = 0;
    for (i = 0; i < 6 && value > orders[i + 1].factor; ++i)
        value /= orders[i + 1].factor;
    std::stringstream oss;
    if (precise) {
        oss << std::fixed << std::setprecision(5) << value << orders[i].suffix;
    } else {
        oss << std::fixed << std::setprecision(3) << value << orders[i].suffix;
    }
    return oss.str();
}

std::string mem_string(size_t size, bool precise) {
    const char *orders[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB" };
    float value          = (float) size;

    int i = 0;
    for (i = 0; i < 6 && value > 1024.f; ++i)
        value /= 1024.f;
    std::stringstream oss;
    if (precise) {
        oss << std::fixed << std::setprecision(5) << value << orders[i];
    } else {
        oss << std::fixed << std::setprecision(3) << value << orders[i];
    }
    return oss.str();
}

} // namespace aspirin
