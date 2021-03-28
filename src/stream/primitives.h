#pragma once

#include "crypto/types.h"
#include "serialize.h"

namespace serialize
{
  namespace primitive
  {
    inline std::istream &operator>>(std::istream &i, int8_t &v) { return varint(i, v); }
    inline std::istream &operator>>(std::istream &i, uint8_t &v) { return varint(i, v); }
    inline std::istream &operator>>(std::istream &i, int16_t &v) { return varint(i, v); }
    inline std::istream &operator>>(std::istream &i, uint16_t &v) { return varint(i, v); }
    inline std::istream &operator>>(std::istream &i, int32_t &v) { return varint(i, v); }
    inline std::istream &operator>>(std::istream &i, uint32_t &v) { return varint(i, v); }
    inline std::istream &operator>>(std::istream &i, int64_t &v) { return varint(i, v); }
    inline std::istream &operator>>(std::istream &i, uint64_t &v) { return varint(i, v); }
    inline std::istream &operator>>(std::istream &i, bool &v) { return serialize(i, v); }

    inline std::ostream &operator<<(std::ostream &o, const int8_t &v) { return varint(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const uint8_t &v) { return varint(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const int16_t &v) { return varint(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const uint16_t &v) { return varint(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const int32_t &v) { return varint(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const uint32_t &v) { return varint(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const int64_t &v) { return varint(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const uint64_t &v) { return varint(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const bool &v) { return serialize(o, v); }
  }
}
