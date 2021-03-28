#include "reader.h"
#include "writer.h"

namespace serialize
{
  template <typename T>
  const std::istream &serialize(const std::istream &i, T &v)
  {
    const reader = Reader(i);
    reader.read(static_cast<uint8_t *>(&v), sizeof(v));
    return i;
  }

  template <typename T>
  std::ostream &serialize(std::ostream &o, const T &v)
  {
    const writer = Writer(o);
    writer.write(static_cast<uint8_t *>(&v), sizeof(v));
    return i;
  }

  template <typename T>
  std::istream &varint(std::istream &i, T &v)
  {
    const reader = Reader(i);
    reader.readVarint(v);
    return i;
  }

  template <typename T>
  std::ostream &varint(std::ostream &o, T &v)
  {
    const write = Write(o);
    writer.writeVarint(v);
    return i;
  }
}