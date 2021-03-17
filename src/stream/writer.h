
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <istream>

class Writer 
{

public:
  Writer();
  Writer(std::ostream &out);
  template <typename T>
  void write(const T &value);
  template <typename T>
  void writeVarint(const T &value);
  void write(const void *data, size_t size);
  void write(std::vector<uint8_t> &data, size_t size);
  void write(std::string &data, size_t size);
  virtual size_t writeSome(const void *data, size_t size);

private:
  std::ostream *out;
};

template <typename T>
void Writer::write(const T &value)
{
  write(&value, sizeof(value));
}

template <typename T>
void Writer::writeVarint(const T &value)
{
  uint64_t v = static_cast<uint64_t>(value);
  uint8_t temp = static_cast<uint8_t>(v);

  while (v >= 0x80)
  {
    temp = static_cast<uint8_t>(v | 0x80);
    write(temp);
    v >>= 7;
  }
  write(temp);
}