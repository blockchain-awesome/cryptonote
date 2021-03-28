
#pragma once

#include "crypto/types.h"
#include "serialize.h"

namespace serialize
{
  namespace crypto
  {
    inline std::istream &operator>>(std::istream &i, const public_key_t &v) { return serialize(i, v); }
    inline std::istream &operator>>(std::istream &i, const secret_key_t &v) { return serialize(i, v); }
    inline std::istream &operator>>(std::istream &i, const key_derivation_t &v) { return serialize(i, v); }
    inline std::istream &operator>>(std::istream &i, const key_image_t &v) { return serialize(i, v); }
    inline std::istream &operator>>(std::istream &i, const signature_t &v) { return serialize(i, v); }
    inline std::istream &operator>>(std::istream &i, const hash_t &v) { return serialize(i, v); }
    inline std::istream &operator>>(std::istream &i, const chacha_iv_t &v) { return serialize(i, v); }
    inline std::istream &operator>>(std::istream &i, const elliptic_curve_scalar_t &v) { return serialize(i, v); }
    inline std::istream &operator>>(std::istream &i, const elliptic_curve_point_t &v) { return serialize(i, v); }

    inline std::ostream &operator<<(std::ostream &o, const public_key_t &v) { return serialize(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const secret_key_t &v) { return serialize(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const key_derivation_t &v) { return serialize(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const key_image_t &v) { return serialize(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const signature_t &v) { return serialize(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const hash_t &v) { return serialize(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const chacha_iv_t &v) { return serialize(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const elliptic_curve_scalar_t &v) { return serialize(o, v); }
    inline std::ostream &operator<<(std::ostream &o, const elliptic_curve_scalar_t &v) { return serialize(o, v); }
  }
}
