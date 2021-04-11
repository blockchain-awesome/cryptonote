
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <istream>
#include <sstream>

class Writer
{

public:
  Writer(std::ostream &out);
  template <typename T>
  void write(const T &value);
  template <typename T>
  void writeVarint(const T &value);
  void write(const void *data, size_t size);
  void write(std::vector<uint8_t> &data, size_t size);
  void write(std::string &data, size_t size);
  virtual size_t writeSome(const void *data, size_t size);

  template <typename T>
  friend std::ostringstream &write(const T &value);

private:
  std::ostream &out;
};

template <typename T>
void Writer::write(const T &value)
{
  write((const void *)&value, sizeof(value));
}

template <typename T>
void Writer::writeVarint(const T &value)
{
  uint64_t v = static_cast<uint64_t>(value);
  while (v >= 0x80)
  {
    uint8_t tag = static_cast<uint8_t>(v | 0x80);

    write(tag);
    v >>= 7;
  }

  uint8_t tag = static_cast<uint8_t>(v);

  write(tag);
}

template <typename T>
void write(const T &t, std::ostringstream &oss)
{
  uint64_t v = t;
  while (v >= 0x80)
  {
    uint8_t tag = (static_cast<char>(v) & 0x7f) | 0x80;
    oss << tag;
    v >>= 7;
  }
  uint8_t tag = static_cast<char>(v);
  oss << tag;
}

inline Writer &operator<<(Writer &o, const int8_t &v)
{
  o.writeVarint(v);
  return o;
}
inline Writer &operator<<(Writer &o, const uint8_t &v)
{
  o.writeVarint(v);
  return o;
}
inline Writer &operator<<(Writer &o, const int16_t &v)
{
  o.writeVarint(v);
  return o;
}
inline Writer &operator<<(Writer &o, const uint16_t &v)
{
  o.writeVarint(v);
  return o;
}
inline Writer &operator<<(Writer &o, const int32_t &v)
{
  o.writeVarint(v);
  return o;
}
inline Writer &operator<<(Writer &o, const uint32_t &v)
{
  o.writeVarint(v);
  return o;
}
inline Writer &operator<<(Writer &o, const int64_t &v)
{
  o.writeVarint(v);
  return o;
}
inline Writer &operator<<(Writer &o, const uint64_t &v)
{
  o.writeVarint(v);
  return o;
}

inline Writer &operator<<(Writer &o, const bool &v)
{
  o.write((const void *)&v, sizeof(v));
  return o;
}

Writer &operator<<(Writer &o, const std::string &v);
