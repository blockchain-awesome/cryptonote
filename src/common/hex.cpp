
#include "hex.h"

namespace hex
{
inline bool isRange(char c, char start, char end)
{
  if ((c >= start) && (c <= end))
  {
    return true;
  }
  return false;
}

uint8_t isHex(char c)
{
  if (isRange(c, '0', '9')) {
    return c - '0';
  }
  
  if (isRange(c, 'a', 'f')) {
    return 10 + c - 'a';
  }
  
  if (isRange(c, 'A', 'F')) {
    return 10 + c - 'A';
  }
  return -1;
}

uint8_t fromString(char c)
{
  uint8_t v = isHex(c);
  if (v != 0xFF)
  {
    return v;
  }
  throw std::runtime_error("fromString: invalid character");
}

bool fromString(char c, uint8_t &value)
{
  uint8_t v = isHex(c);
  if (v == 0xFF)
  {
    return false;
  }

  value = v;
  return true;
}

size_t fromString(const std::string &text, void *data, size_t bufferSize)
{
  if ((text.size() & 1) != 0)
  {
    throw std::runtime_error("fromString: invalid string size");
  }

  if (text.size() >> 1 > bufferSize)
  {
    throw std::runtime_error("fromString: invalid buffer size");
  }

  for (size_t i = 0; i < text.size() >> 1; ++i)
  {
    static_cast<uint8_t *>(data)[i] = fromString(text[i << 1]) << 4 | fromString(text[(i << 1) + 1]);
  }

  return text.size() >> 1;
}

bool fromString(const std::string &text, void *data, size_t bufferSize, size_t &size)
{
  if ((text.size() & 1) != 0)
  {
    return false;
  }

  if (text.size() >> 1 > bufferSize)
  {
    return false;
  }

  for (size_t i = 0; i < text.size() >> 1; ++i)
  {
    uint8_t value1;
    if (!fromString(text[i << 1], value1))
    {
      return false;
    }

    uint8_t value2;
    if (!fromString(text[(i << 1) + 1], value2))
    {
      return false;
    }

    static_cast<uint8_t *>(data)[i] = value1 << 4 | value2;
  }

  size = text.size() >> 1;
  return true;
}

binary_array_t fromString(const std::string &text)
{
  if ((text.size() & 1) != 0)
  {
    throw std::runtime_error("fromString: invalid string size");
  }

  binary_array_t data(text.size() >> 1);
  for (size_t i = 0; i < data.size(); ++i)
  {
    data[i] = fromString(text[i << 1]) << 4 | fromString(text[(i << 1) + 1]);
  }

  return data;
}

bool fromString(const std::string &text, binary_array_t &data)
{
  if ((text.size() & 1) != 0)
  {
    return false;
  }

  for (size_t i = 0; i<text.size()>> 1; ++i)
  {
    uint8_t value1;
    if (!fromString(text[i << 1], value1))
    {
      return false;
    }

    uint8_t value2;
    if (!fromString(text[(i << 1) + 1], value2))
    {
      return false;
    }

    data.push_back(value1 << 4 | value2);
  }

  return true;
}

std::string toString(const void* data, size_t size) {
  std::string text;
  for (size_t i = 0; i < size; ++i) {
    text += "0123456789abcdef"[static_cast<const uint8_t*>(data)[i] >> 4];
    text += "0123456789abcdef"[static_cast<const uint8_t*>(data)[i] & 0xF];
  }

  return text;
}

void toString(const void* data, size_t size, std::string& text) {
  for (size_t i = 0; i < size; ++i) {
    text += "0123456789abcdef"[static_cast<const uint8_t*>(data)[i] >> 4];
    text += "0123456789abcdef"[static_cast<const uint8_t*>(data)[i] & 15];
  }
}

std::string toString(const binary_array_t& data) {
  std::string text;
  for (size_t i = 0; i < data.size(); ++i) {
    text += "0123456789abcdef"[data[i] >> 4];
    text += "0123456789abcdef"[data[i] & 15];
  }

  return text;
}

void toString(const binary_array_t& data, std::string& text) {
  for (size_t i = 0; i < data.size(); ++i) {
    text += "0123456789abcdef"[data[i] >> 4];
    text += "0123456789abcdef"[data[i] & 15];
  }
}
} // namespace hex