
#include "binary_array.h"

namespace array
{
std::string toString(const void *data, size_t size)
{
  return std::string(static_cast<const char *>(data), size);
}

std::string toString(const binary_array_t &data)
{
  return std::string(reinterpret_cast<const char *>(data.data()), data.size());
}

binary_array_t fromString(const std::string& data) {
  auto dataPtr = reinterpret_cast<const uint8_t*>(data.data());
  return binary_array_t(dataPtr, dataPtr + data.size());
}
} // namespace array
