#pragma once

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "binary.h"

namespace hex
{
  uint8_t from(char character);
  bool from(char character, uint8_t &value);

  bool from(const std::string &text, void *data, size_t bufferSize); // Assigns values of hex 'text' to buffer 'data' up to 'bufferSize', assigns actual data size to 'size', returns false on error, does not throw
  binary_array_t from(const std::string &text);                                    // Returns values of hex 'text', throws on error
  bool from(const std::string &text, binary_array_t &data);                        // Appends values of hex 'text' to 'data', returns false on error, does not throw

  template <typename T>
  bool podFrom(const std::string &text, T &val)
  {    
    return  from(text, &val, sizeof(val)) && (text.size() >> 1) == sizeof(val);
  }

  std::string to(const void *data, size_t size);             // Returns hex representation of ('data', 'size'), does not throw
  void to(const void *data, size_t size, std::string &text); // Appends hex representation of ('data', 'size') to 'text', does not throw
  std::string to(const binary_array_t &data);                // Returns hex representation of 'data', does not throw
  void to(const binary_array_t &data, std::string &text);    // Appends hex representation of 'data' to 'text', does not throw

  template <class T>
  std::string podTo(const T &s)
  {
    return to(&s, sizeof(s));
  }
} // namespace hex