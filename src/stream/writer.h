
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <istream>

class Writer 
{

public:
  Writer(std::ostream &out);
  template <typename T>
  void write(T &value);
  template <typename T>
  void writeVarint(T &value);
  void write(const void *data, size_t size);
  void write(std::vector<uint8_t> &data, size_t size);
  void write(std::string &data, size_t size);
  virtual size_t writeSome(const void *data, size_t size);

private:
  std::ostream &out;
};

template <typename T>
void Writer::write(T &value)
{
  write(&value, sizeof(value));
}

template <typename T>
void Writer::writeVarint(T &value)
{
  uint8_t temp = static_cast<uint8_t>(value);

  while (value >= 0x80)
  {
    temp = static_cast<uint8_t>(value | 0x80);
    write(temp);
    value >>= 7;
  }

  write(temp);
}