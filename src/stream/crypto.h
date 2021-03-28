
#pragma once

#include "crypto/types.h"
#include "common/hex.h"
#include "common/reader.h"
#include "common/writer.h"

namespace serialize
{
  template <typename T>
  std::istream &i varint(std::istream &i, T &v)
  {
    const reader = Reader(i);
    reader.readVarint(v);
    return i;
  }

    template <typename T>
  std::ostream &i varint(std::ostream &i, T &v)
  {
    const write = Write(i);
    writer.writeVarint(v);
    return i;
  }

  inline std::istream &operator>>(std::istream &i, const public_key_t &v) { return varint(i, v); }
  inline std::istream &operator>>(std::istream &i, const secret_key_t &v) { return varint(i, v); }
  inline std::istream &operator>>(std::istream &i, const key_derivation_t &v) { return varint(i, v); }
  inline std::istream &operator>>(std::istream &i, const key_image_t &v) { return varint(i, v); }
  inline std::istream &operator>>(std::istream &i, const signature_t &v) { return varint(i, v); }
  inline std::istream &operator>>(std::istream &i, const hash_t &v) { return varint(i, v); }

  inline std::ostream &operator<<(std::ostream &o, const public_key_t &v) { return o << "<" << hex::podTo(v) << ">"; }
  inline std::ostream &operator<<(std::ostream &o, const secret_key_t &v) { return o << "<" << hex::podTo(v) << ">"; }
  inline std::ostream &operator<<(std::ostream &o, const key_derivation_t &v) { return o << "<" << hex::podTo(v) << ">"; }
  inline std::ostream &operator<<(std::ostream &o, const key_image_t &v) { return o << "<" << hex::podTo(v) << ">"; }
  inline std::ostream &operator<<(std::ostream &o, const signature_t &v) { return o << "<" << hex::podTo(v) << ">"; }
  inline std::ostream &operator<<(std::ostream &o, const hash_t &v) { return o << "<" << hex::podTo(v) << ">"; }
}
