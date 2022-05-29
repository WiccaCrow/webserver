#pragma once

#include "Header.hpp"
#include <stdint.h>

namespace HTTP {

    extern const std::map<uint32_t, Header::Handler> validHeaders;

} // namespace HTTP
