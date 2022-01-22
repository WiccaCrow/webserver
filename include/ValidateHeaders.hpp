#pragma once

#include <stddef.h>

#include <map>

#include "Types.hpp"

namespace HTTP {

extern const std::map<uint32, bool> validHeaders;
extern const size_t                 tableSize;

}; // namespace HTTP