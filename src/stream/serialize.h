#include "reader.h"
#include "writer.h"

namespace serialize
{
  template <typename T>
  std::istream &bytes(std::istream &i, T &v)
  {
    Reader reader = Reader(i);
    reader.read(static_cast<uint8_t *>(&v), sizeof(v));
    return i;
  }

  std::istream &bytes(std::istream &i, void *v, const size_t len)
  {
    Reader reader = Reader(i);
    reader.read(v, len);
    return i;
  }

  template <typename T>
  std::ostream &bytes(std::ostream &o, const T &v)
  {
    Writer writer = Writer(o);
    writer.write(static_cast<uint8_t *>(&v), sizeof(v));
    return o;
  }

  std::ostream &bytes(std::ostream &o, uint8_t *v, const size_t len)
  {
    Writer writer = Writer(o);
    writer.write(v, len);
    return o;
  }

  template <typename T>
  std::istream &varint(std::istream &i, T &v)
  {
    Reader reader = Reader(i);
    reader.readVarint(v);
    return i;
  }

  template <typename T>
  std::ostream &varint(std::ostream &o, T &v)
  {
    Writer writer = Writer(o);
    writer.writeVarint(v);
    return o;
  }

  class IStream
  {
    virtual bool serialize(std::ostream &o) const = 0;
    virtual bool serialize(std::istream &i) const = 0;
    friend std::ostream &operator>>(std::ostream &o, const IStream &value);
    friend std::istream &operator<<(std::istream &o, const IStream &value);
  };

  std::ostream &operator>>(std::ostream &o, const IStream &value)
  {
    value.serialize(o);
    return o;
  };

  std::istream &operator<<(std::istream &i, const IStream &value)
  {
    value.serialize(i);
    return i;
  };
}