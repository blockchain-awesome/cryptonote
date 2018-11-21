#pragma once

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "binary_array.h"

namespace hex
{
uint8_t toString(char character);                                                    // Returns value of hex 'character', throws on error
bool toString(char character, uint8_t &value);                                       // Assigns value of hex 'character' to 'value', returns false on error, does not throw
size_t toString(const std::string &text, void *data, size_t bufferSize);             // Assigns values of hex 'text' to buffer 'data' up to 'bufferSize', returns actual data size, throws on error
bool toString(const std::string &text, void *data, size_t bufferSize, size_t &size); // Assigns values of hex 'text' to buffer 'data' up to 'bufferSize', assigns actual data size to 'size', returns false on error, does not throw
binary_array_t toString(const std::string &text);                                    // Returns values of hex 'text', throws on error
bool toString(const std::string &text, binary_array_t &data);                        // Appends values of hex 'text' to 'data', returns false on error, does not throw

template <typename T>
bool podToString(const std::string &text, T &val)
{
  size_t outSize;
  return toString(text, &val, sizeof(val), outSize) && outSize == sizeof(val);
}
} // namespace hex