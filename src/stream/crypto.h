
#pragma once

#include "crypto/types.h"
#include "reader.h"
#include "writer.h"

#define ISTREAMOP(T)                         \
  inline Reader &operator>>(Reader &i, T &v) \
  {                                          \
    i.read(v);                               \
    return i;                                \
  }

#define OSTREAMOP(T)                         \
  inline Writer &operator<<(Writer &o, T &v) \
  {                                          \
    o.write(v);                              \
    return o;                                \
  }

ISTREAMOP(public_key_t);
ISTREAMOP(secret_key_t);
ISTREAMOP(key_derivation_t);
ISTREAMOP(key_image_t);
ISTREAMOP(signature_t);
ISTREAMOP(hash_t);
ISTREAMOP(chacha_iv_t);
ISTREAMOP(elliptic_curve_scalar_t);
ISTREAMOP(elliptic_curve_point_t);

OSTREAMOP(public_key_t);
OSTREAMOP(secret_key_t);
OSTREAMOP(key_derivation_t);
OSTREAMOP(key_image_t);
OSTREAMOP(signature_t);
OSTREAMOP(hash_t);
OSTREAMOP(chacha_iv_t);
OSTREAMOP(elliptic_curve_scalar_t);
OSTREAMOP(elliptic_curve_point_t);
