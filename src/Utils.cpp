#include "Utils.hpp"

std::string to_string(int val) {
    char buf[25];
    snprintf(buf, 25, "%d", val);
    return std::string(buf);
}