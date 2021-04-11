

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <istream>

struct membuf : std::streambuf
{
  membuf(char *begin, char *end)
  {
    this->setg(begin, begin, end);
  }
};

class Reader
{

public:
  Reader(std::istream &in);
  template <typename T>
  void read(T &value);
  template <typename T>
  void readVarint(T &value);
  void read(void *data, size_t size);

  void readHeight(size_t &height);
  virtual size_t readSome(void *data, size_t size);
  void read(std::vector<uint8_t> &data, size_t size);
  void read(std::string &data, size_t size);
  size_t getPosition() const;
  bool endOfStream() const;

protected:
  std::istream &in;
};

template <typename T>
void Reader::read(T &value)
{
  read(&value, sizeof(value));
}

template <typename T>
void Reader::readVarint(T &value)
{
  T temp = 0;
  for (uint8_t shift = 0;; shift += 7)
  {
    uint8_t piece;
    read(piece);
    if (shift >= sizeof(temp) * 8 - 7 && piece >= 1 << (sizeof(temp) * 8 - shift))
    {
      throw std::runtime_error("readVarint, value overflow");
    }

    temp |= static_cast<size_t>(piece & 0x7f) << shift;
    if ((piece & 0x80) == 0)
    {
      if (piece == 0 && shift != 0)
      {
        throw std::runtime_error("readVarint, invalid value representation");
      }

      break;
    }
  }

  value = temp;
}

inline Reader &operator>>(Reader &i, int8_t &v)
{
  i.readVarint(v);
  return i;
}
inline Reader &operator>>(Reader &i, uint8_t &v)
{
  i.readVarint(v);
  return i;
}
inline Reader &operator>>(Reader &i, int16_t &v)
{
  i.readVarint(v);
  return i;
}
inline Reader &operator>>(Reader &i, uint16_t &v)
{
  i.readVarint(v);
  return i;
}
inline Reader &operator>>(Reader &i, int32_t &v)
{
  i.readVarint(v);
  return i;
}
inline Reader &operator>>(Reader &i, uint32_t &v)
{
  i.readVarint(v);
  return i;
}
inline Reader &operator>>(Reader &i, int64_t &v)
{
  i.readVarint(v);
  return i;
}
inline Reader &operator>>(Reader &i, uint64_t &v)
{
  i.readVarint(v);
  return i;
}

inline Reader &operator>>(Reader &i, bool &v)
{
  i.read((void *)&v, sizeof(v));
  return i;
}

Reader &operator>>(Reader &i, std::string &v);


