
#pragma once

#include "crypto/types.h"
#include "serialize.h"

namespace stream
{
  namespace crypto
  {
    inline vistream &operator>>(vistream &i, public_key_t &v) { return bytes(i, v); }
    inline vistream &operator>>(vistream &i, secret_key_t &v) { return bytes(i, v); }
    inline vistream &operator>>(vistream &i, key_derivation_t &v) { return bytes(i, v); }
    inline vistream &operator>>(vistream &i, key_image_t &v) { return bytes(i, v); }
    inline vistream &operator>>(vistream &i, signature_t &v) { return bytes(i, v); }
    inline vistream &operator>>(vistream &i, hash_t &v) { return bytes(i, v); }
    inline vistream &operator>>(vistream &i, chacha_iv_t &v) { return bytes(i, v); }
    inline vistream &operator>>(vistream &i, elliptic_curve_scalar_t &v) { return bytes(i, v); }
    inline vistream &operator>>(vistream &i, elliptic_curve_point_t &v) { return bytes(i, v); }

    inline vostream &operator<<(vostream &o, const public_key_t &v) { return bytes(o, v); }
    inline vostream &operator<<(vostream &o, const secret_key_t &v) { return bytes(o, v); }
    inline vostream &operator<<(vostream &o, const key_derivation_t &v) { return bytes(o, v); }
    inline vostream &operator<<(vostream &o, const key_image_t &v) { return bytes(o, v); }
    inline vostream &operator<<(vostream &o, const signature_t &v) { return bytes(o, v); }
    inline vostream &operator<<(vostream &o, const hash_t &v) { return bytes(o, v); }
    inline vostream &operator<<(vostream &o, const chacha_iv_t &v) { return bytes(o, v); }
    inline vostream &operator<<(vostream &o, const elliptic_curve_scalar_t &v) { return bytes(o, v); }
    inline vostream &operator<<(vostream &o, const elliptic_curve_scalar_t &v) { return bytes(o, v); }
  }
}
