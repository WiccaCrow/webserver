#pragma once

#include <stdint.h>

#include "Header.hpp"

namespace HTTP {

extern const std::map<uint32_t, Header::Handler> validHeaders;

} // namespace HTTP
