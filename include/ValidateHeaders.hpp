#pragma once

#include <stddef.h>

#include <map>

#include "Header.hpp"
#include "HeadersCodes.hpp"

namespace HTTP {

extern const std::map<uint32_t, bool> validHeaders;

}; // namespace HTTP