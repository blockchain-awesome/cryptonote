#pragma once

#include <istream>
#include <ostream>

namespace stream
{

  class vistream
  {
  public:
    vistream(std::istream &i) : i(i) {}
    std::istream &i;
  };

  class vostream
  {
  public:
    vostream(std::ostream &o) : o(o) {}
    std::ostream &o;
  };

  template <typename T>
  vistream &bytes(vistream &i, T &v)
  {
    i.i.read(static_cast<char *>(&v), sizeof(v));
    return i;
  }

  vistream &bytes(vistream &i, char *v, const size_t len);

  template <typename T>
  vostream &bytes(vostream &o, const T &v)
  {
    o.o.write(static_cast<char *>(&v), sizeof(v));
    return o;
  }

  vostream &bytes(vostream &o, const char *v, const size_t len);

  template <typename T>
  vistream &varint(vistream &i, T &v)
  {
    T temp = 0;
    for (uint8_t shift = 0;; shift += 7)
    {
      uint8_t piece;
      i.i.read((char *)(&piece), 1);
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

    v = temp;
    return i;
  }

  template <typename T>
  vostream &varint(vostream &o, T &t)
  {
    uint64_t v = t;
    while (v >= 0x80)
    {
      uint8_t tag = (static_cast<char>(v) & 0x7f) | 0x80;
      o.o << tag;
      v >>= 7;
    }
    uint8_t tag = static_cast<char>(v);
    o.o << tag;
    return o;
  }

}