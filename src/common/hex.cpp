
#include "hex.h"

namespace hex
{

  char radix[] = "0123456789abcdef";

  inline bool within(char c, char start, char end)
  {
    return (c >= start) && (c <= end);
  }

  uint8_t convert(char c)
  {
    uint8_t range[][3] = {{'0', '9', 0}, {'a', 'f', 10}, {'A', 'F', 10}};
    for (int i = 0; i < 3; i++)
    {
      if (within(c, range[i][0], range[i][1]))
      {
        return c - range[i][0] + range[i][2];
      }
    }
    return -1;
  }

  uint8_t from(char c)
  {
    uint8_t v = convert(c);
    if (v != 0xFF)
    {
      return v;
    }
    throw std::runtime_error("from: invalid character");
  }

  bool from(char c, uint8_t &value)
  {
    uint8_t v = convert(c);
    if (v == 0xFF)
    {
      return false;
    }

    value = v;
    return true;
  }

  bool from(const std::string &text, void *data, size_t bufferSize)
  {
    if ((text.size() & 1) != 0)
    {
      return false;
    }

    const size_t size = text.size() >> 1;

    if (size > bufferSize)
    {
      return false;
    }

    for (size_t i = 0; i < size; ++i)
    {
      uint8_t value1;
      if (!from(text[i << 1], value1))
      {
        return false;
      }

      uint8_t value2;
      if (!from(text[(i << 1) + 1], value2))
      {
        return false;
      }

      static_cast<uint8_t *>(data)[i] = value1 << 4 | value2;
    }

    return true;
  }

  binary_array_t from(const std::string &text)
  {
    if ((text.size() & 1) != 0)
    {
      throw std::runtime_error("from: invalid string size!");
    }

    binary_array_t data(text.size() >> 1);
    for (size_t i = 0; i < data.size(); ++i)
    {
      data[i] = from(text[i << 1]) << 4 | from(text[(i << 1) + 1]);
    }

    return data;
  }

  bool from(const std::string &text, binary_array_t &data)
  {
    if ((text.size() & 1) != 0)
    {
      return false;
    }

    for (size_t i = 0; i < text.size() >> 1; ++i)
    {
      uint8_t value1;
      if (!from(text[i << 1], value1))
      {
        return false;
      }

      uint8_t value2;
      if (!from(text[(i << 1) + 1], value2))
      {
        return false;
      }

      data.push_back(value1 << 4 | value2);
    }

    return true;
  }

  void to(const void *data, size_t size, std::string &text)
  {
    const uint8_t *addr = (const uint8_t *)data;
    for (size_t i = 0; i < size; ++i)
    {
      text += radix[(addr)[i] >> 4];
      text += radix[(addr)[i] & 0xF];
    }
  }

  std::string to(const void *data, size_t size)
  {
    std::string text;

    to(data, size, text);

    return text;
  }

  std::string to(const binary_array_t &data)
  {
    std::string text;

    to(&data[0], data.size(), text);

    return text;
  }

  void to(const binary_array_t &data, std::string &text)
  {
    to(&data[0], data.size(), text);
  }
} // namespace hex