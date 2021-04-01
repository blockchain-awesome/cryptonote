#include <istream>
#include <ostream>

namespace serialize
{
  template <typename T>
  std::istream &bytes(std::istream &i, T &v)
  {
    i.read(static_cast<char *>(&v), sizeof(v));
    return i;
  }

  std::istream &bytes(std::istream &i, char *v, const size_t len)
  {
    i.read(v, len);
    return i;
  }

  template <typename T>
  std::ostream &bytes(std::ostream &o, const T &v)
  {
    o.write(static_cast<char *>(&v), sizeof(v));
    return o;
  }

  std::ostream &bytes(std::ostream &o, const char *v, const size_t len)
  {
    o.write(v, len);
    return o;
  }

  template <typename T>
  std::istream &varint(std::istream &i, T &v)
  {
    T temp = 0;
    for (uint8_t shift = 0;; shift += 7)
    {
      uint8_t piece;
      i.read((char *)(&piece), 1);
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
  std::ostream &varint(std::ostream &o, T &t)
  {
    uint64_t v = t;
    while (v >= 0x80)
    {
      uint8_t tag = (static_cast<char>(v) & 0x7f) | 0x80;
      o << tag;
      v >>= 7;
    }
    uint8_t tag = static_cast<char>(v);
    o << tag;
    return o;
  }

  class IStream
  {
    virtual bool serialize(std::ostream &o) = 0;
    virtual bool serialize(std::istream &i) = 0;
    friend std::ostream &operator<<(std::ostream &o, IStream &value);
    friend std::istream &operator>>(std::istream &o, IStream &value);
  };

  std::ostream &operator<<(std::ostream &o, IStream &value)
  {
    value.serialize(o);
    return o;
  };

  std::istream &operator>>(std::istream &i, IStream &value)
  {
    value.serialize(i);
    return i;
  };
}