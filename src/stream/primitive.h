#pragma once

#include "crypto/types.h"
#include "serialize.h"

namespace stream
{
  namespace primitive
  {
    inline vistream &operator>>(vistream &i, int8_t &v) { return varint(i, v); }
    inline vistream &operator>>(vistream &i, uint8_t &v) { return varint(i, v); }
    inline vistream &operator>>(vistream &i, int16_t &v) { return varint(i, v); }
    inline vistream &operator>>(vistream &i, uint16_t &v) { return varint(i, v); }
    inline vistream &operator>>(vistream &i, int32_t &v) { return varint(i, v); }
    inline vistream &operator>>(vistream &i, uint32_t &v) { return varint(i, v); }
    inline vistream &operator>>(vistream &i, int64_t &v) { return varint(i, v); }
    inline vistream &operator>>(vistream &i, uint64_t &v) { return varint(i, v); }
    // inline vistream &operator>>(vistream &i, size_t &v) { return varint(i, v); }
    inline vistream &operator>>(vistream &i, bool &v) { return bytes(i, (char *)&v, sizeof(v)); }

    inline vostream &operator<<(vostream &o, const int8_t &v) { return varint(o, v); }
    inline vostream &operator<<(vostream &o, const uint8_t &v) { return varint(o, v); }
    inline vostream &operator<<(vostream &o, const int16_t &v) { return varint(o, v); }
    inline vostream &operator<<(vostream &o, const uint16_t &v) { return varint(o, v); }
    inline vostream &operator<<(vostream &o, const int32_t &v) { return varint(o, v); }
    inline vostream &operator<<(vostream &o, const uint32_t &v) { return varint(o, v); }
    inline vostream &operator<<(vostream &o, const int64_t &v) { return varint(o, v); }
    inline vostream &operator<<(vostream &o, const uint64_t &v) { return varint(o, v); }
    // inline vostream &operator<<(vostream &o, const size_t &v) { return varint(o, v); }
    inline vostream &operator<<(vostream &o, const bool &v) { return bytes(o, (char *)&v, sizeof(v)); }
  }
}
