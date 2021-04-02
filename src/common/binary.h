#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include <limits>

typedef std::vector<uint8_t> binary_array_t;

class IBinary
{
  IBinary();

public:
  virtual void read(std::istream &is) = 0;
  virtual void write(std::ostream &os) const = 0;

  static std::string to(const void *data, size_t size);
  static std::string to(const binary_array_t &data);
  static binary_array_t from(const std::string &data);

  binary_array_t to();
  void from(const binary_array_t &blob);
  bool save(const std::string &filename);

  bool load(const std::string &filename);

  friend std::istream &operator>>(std::istream &is, IBinary &base);
  friend std::ostream &operator<<(std::ostream &os, const IBinary &base);

  template <typename T>
  friend std::string fromVarint(T &t);
};

template <typename T>
std::string fromVarint(T &t)
{
  std::ostringstream oss;
  uint64_t v = t;
  while (v >= 0x80)
  {
    uint8_t tag = (static_cast<char>(v) & 0x7f) | 0x80;
    oss << tag;
    v >>= 7;
  }
  uint8_t tag = static_cast<char>(v);
  oss << tag;

  return oss.str();
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, int>::type
toVarint(std::string &str, T &i)
{
  std::string::iterator first = std::move(str.begin());
  std::string::iterator last = std::move(str.end());
  int bits = std::numeric_limits<T>::digits;
  int read = 0;
  i = 0;
  for (int shift = 0;; shift += 7)
  {
    if (first == last)
    {
      return read; // End of input.
    }
    unsigned char byte = *first++;
    ++read;
    if (shift + 7 >= bits && byte >= 1 << (bits - shift))
    {
      return -1; // Overflow.
    }
    if (byte == 0 && shift != 0)
    {
      return -2; // Non-canonical representation.
    }
    i |= static_cast<T>(byte & 0x7f) << shift;
    if ((byte & 0x80) == 0)
    {
      break;
    }
  }
  return read;
}

namespace binary
{
  extern bool load(const std::string &filepath, std::string &buf);
  extern bool save(const std::string &filepath, const std::string &buf);
  namespace is
  {
    template <typename T>
    T from(const std::string &text)
    {
      T value;
      std::istringstream ss(text);
      ss >> value;
      if (ss.fail())
      {
        throw std::runtime_error("fromString: unable to parse value");
      }

      return value;
    }

    template <typename T>
    bool from(const std::string &text, T &value)
    {
      std::istringstream s(text);
      s >> value;
      return !s.fail();
    }

  }

  namespace os
  {

    template <typename T>
    std::string to(const T &value)
    {
      std::ostringstream s;
      s << value;
      return s.str();
    }

    template <typename T>
    void to(const T &value, std::string &text)
    {
      std::ostringstream s;
      s << value;
      text += s.str();
    }

    template <typename T>
    std::string from(T i)
    {
      std::ostringstream oss;
      oss << i;
      return oss.str();
    }
  }
}