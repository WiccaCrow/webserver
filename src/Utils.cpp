#include "Utils.hpp"

std::string to_string(int val) {
    char buf[25];
    int  got_len = snprintf(buf, 25, "%d", val);
    return std::string(buf);
}