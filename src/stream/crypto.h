
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
  inline Writer &operator>>(Writer &o, T &v) \
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

// inline Reader &operator>>(Reader &i, public_key_t &v)
// {
//   i.read(v);
//   return i;
// }
// inline Reader &operator>>(Reader &i, secret_key_t &v)
// {
//   i.read(v);
//   return i;
// }
// inline Reader &operator>>(Reader &i, key_derivation_t &v)
// {
//   i.read(v);
//   return i;
// }
// inline Reader &operator>>(Reader &i, key_image_t &v)
// {
//   i.read(v);
//   return i;
// }
// inline Reader &operator>>(Reader &i, signature_t &v)
// {
//   i.read(v);
//   return i;
// }
// inline Reader &operator>>(Reader &i, hash_t &v)
// {
//   i.read(v);
//   return i;
// }
// inline Reader &operator>>(Reader &i, chacha_iv_t &v)
// {
//   i.read(v);
//   return i;
// }
// inline Reader &operator>>(Reader &i, elliptic_curve_scalar_t &v)
// {
//   i.read(v);
//   return i;
// }
// inline Reader &operator>>(Reader &i, elliptic_curve_point_t &v)
// {
//   i.read(v);
//   return i;
// }

// inline Writer &operator<<(Writer &o, const public_key_t &v)
// {
//   o.write(v);
//   return o;
// }
// inline Writer &operator<<(Writer &o, const secret_key_t &v)
// {
//   o.write(v);
//   return o;
// }
// inline Writer &operator<<(Writer &o, const key_derivation_t &v)
// {
//   o.write(v);
//   return o;
// }
// inline Writer &operator<<(Writer &o, const key_image_t &v)
// {
//   o.write(v);
//   return o;
// }
// inline Writer &operator<<(Writer &o, const signature_t &v)
// {
//   o.write(v);
//   return o;
// }
// inline Writer &operator<<(Writer &o, const hash_t &v)
// {
//   o.write(v);
//   return o;
// }
// inline Writer &operator<<(Writer &o, const chacha_iv_t &v)
// {
//   o.write(v);
//   return o;
// }
// inline Writer &operator<<(Writer &o, const elliptic_curve_scalar_t &v)
// {
//   o.write(v);
//   return o;
// }
// inline Writer &operator<<(Writer &o, const elliptic_curve_point_t &v)
// {
//   o.write(v);
//   return o;
// }
