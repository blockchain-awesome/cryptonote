
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
} // namespace array
