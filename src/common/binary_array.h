#pragma once

#include <vector>
#include <cstdint>
#include <string>

typedef std::vector<uint8_t> binary_array_t;
namespace array
{
std::string toString(const void *data, size_t size);
std::string toString(const binary_array_t &data);
} // namespace array
