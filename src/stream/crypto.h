
#pragma once

#include "crypto/types.h"
#include "serialize.h"

namespace serialize
{
  namespace crypto
  {
    inline std::istream &operator>>(std::istream &i, public_key_t &v) { return bytes(i, v); }
    inline std::istream &operator>>(std::istream &i, secret_key_t &v) { return bytes(i, v); }
    inline std::istream &operator>>(std::istream &i, key_derivation_t &v) { return bytes(i, v); }
    inline std::istream &operator>>(std::istream &i, key_image_t &v) { return bytes(i, v); }
    inline std::istream &operator>>(std::istream &i, signature_t &v) { return bytes(i, v); }
    inline std::istream &operator>>(std::istream &i, hash_t &v) { return bytes(i, v); }
    inline std::istream &operator>>(std::istream &i, chacha_iv_t &v) { return bytes(i, v); }
    inline std::istream &operator>>(std::istream &i, elliptic_curve_scalar_t &v) { return bytes(i, v); }
    inline std::istream &operator>>(std::istream &i, elliptic_curve_point_t &v) { return bytes(i, v); }

    inline std::ostream &operator<<(std::ostream &o, const public_key_t &v) { return bytes(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const secret_key_t &v) { return bytes(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const key_derivation_t &v) { return bytes(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const key_image_t &v) { return bytes(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const signature_t &v) { return bytes(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const hash_t &v) { return bytes(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const chacha_iv_t &v) { return bytes(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const elliptic_curve_scalar_t &v) { return bytes(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const elliptic_curve_scalar_t &v) { return bytes(o, v); }
  }
}
