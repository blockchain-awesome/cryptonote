
#pragma once

#include "crypto/types.h"
#include "reader.h"
#include "writer.h"

namespace stream
{
  namespace crypto
  {
    inline Reader &operator>>(Reader &i, public_key_t &v) {  i.read(v); return i;}
    inline Reader &operator>>(Reader &i, secret_key_t &v) {  i.read(v); return i; }
    inline Reader &operator>>(Reader &i, key_derivation_t &v) {  i.read(v); return i; }
    inline Reader &operator>>(Reader &i, key_image_t &v) {  i.read(v); return i; }
    inline Reader &operator>>(Reader &i, signature_t &v) {  i.read(v); return i; }
    inline Reader &operator>>(Reader &i, hash_t &v) {  i.read(v); return i; }
    inline Reader &operator>>(Reader &i, chacha_iv_t &v) {  i.read(v); return i; }
    inline Reader &operator>>(Reader &i, elliptic_curve_scalar_t &v) {  i.read(v); return i; }
    inline Reader &operator>>(Reader &i, elliptic_curve_point_t &v) {  i.read(v); return i; }

    inline Writer &operator<<(Writer &o, const public_key_t &v) { o.write(v); return o;}
    inline Writer &operator<<(Writer &o, const secret_key_t &v) { o.write(v); return o; }
    inline Writer &operator<<(Writer &o, const key_derivation_t &v) { o.write(v); return o; }
    inline Writer &operator<<(Writer &o, const key_image_t &v) { o.write(v); return o; }
    inline Writer &operator<<(Writer &o, const signature_t &v) { o.write(v); return o; }
    inline Writer &operator<<(Writer &o, const hash_t &v) { o.write(v); return o; }
    inline Writer &operator<<(Writer &o, const chacha_iv_t &v) { o.write(v); return o; }
    inline Writer &operator<<(Writer &o, const elliptic_curve_scalar_t &v) { o.write(v); return o; }
    inline Writer &operator<<(Writer &o, const elliptic_curve_scalar_t &v) { o.write(v); return o; }
  }
}
