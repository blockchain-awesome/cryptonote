// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <alloca.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>

#include "common/varint.h"
#include "crypto.h"
#include "hash.h"

namespace crypto {

  using std::abort;
  using std::int32_t;
  using std::lock_guard;
  using std::mutex;

  extern "C" {
    #include "crypto-ops.h"
    #include "random.h"
    #include "crypto-defines.h"
  }

  mutex random_lock;

  bool crypto_ops::generate_key_derivation(const public_key_t &key1, const secret_key_t &key2, key_derivation_t &derivation) {
    ge_p3 point;
    ge_p2 point2;
    ge_p1p1 point3;
    assert(sc_check(reinterpret_cast<const unsigned char*>(&key2)) == 0);
    if (ge_frombytes_vartime(&point, reinterpret_cast<const unsigned char*>(&key1)) != 0) {
      return false;
    }
    ge_scalarmult(&point2, reinterpret_cast<const unsigned char*>(&key2), &point);
    ge_mul8(&point3, &point2);
    ge_p1p1_to_p2(&point2, &point3);
    ge_tobytes(reinterpret_cast<unsigned char*>(&derivation), &point2);
    return true;
  }

  static void derivation_to_scalar(const key_derivation_t &derivation, size_t output_index, elliptic_curve_scalar_t &res) {
    struct {
      key_derivation_t derivation;
      char output_index[(sizeof(size_t) * 8 + 6) / 7];
    } buf;
    char *end = buf.output_index;
    buf.derivation = derivation;
    varint::write(end, output_index);
    assert(end <= buf.output_index + sizeof buf.output_index);
    hash_to_scalar((const uint8_t *)&buf, end - reinterpret_cast<char *>(&buf), (uint8_t *)&res);
  }

  static void derivation_to_scalar(const key_derivation_t &derivation, size_t output_index, const uint8_t* suffix, size_t suffixLength, elliptic_curve_scalar_t &res) {
    assert(suffixLength <= 32);
    struct {
      key_derivation_t derivation;
      char output_index[(sizeof(size_t) * 8 + 6) / 7 + 32];
    } buf;
    char *end = buf.output_index;
    buf.derivation = derivation;
    varint::write(end, output_index);
    assert(end <= buf.output_index + sizeof buf.output_index);
    size_t bufSize = end - reinterpret_cast<char *>(&buf);
    memcpy(end, suffix, suffixLength);
    hash_to_scalar((const uint8_t *)&buf, bufSize + suffixLength, (uint8_t *)&res);
  }

  bool crypto_ops::derive_public_key(const key_derivation_t &derivation, size_t output_index,
    const public_key_t &base, public_key_t &derived_key) {
    elliptic_curve_scalar_t scalar;
    ge_p3 point1;
    ge_p3 point2;
    ge_cached point3;
    ge_p1p1 point4;
    ge_p2 point5;
    if (ge_frombytes_vartime(&point1, reinterpret_cast<const unsigned char*>(&base)) != 0) {
      return false;
    }
    derivation_to_scalar(derivation, output_index, scalar);
    ge_scalarmult_base(&point2, reinterpret_cast<unsigned char*>(&scalar));
    ge_p3_to_cached(&point3, &point2);
    ge_add(&point4, &point1, &point3);
    ge_p1p1_to_p2(&point5, &point4);
    ge_tobytes(reinterpret_cast<unsigned char*>(&derived_key), &point5);
    return true;
  }

  bool crypto_ops::derive_public_key(const key_derivation_t &derivation, size_t output_index,
    const public_key_t &base, const uint8_t* suffix, size_t suffixLength, public_key_t &derived_key) {
    elliptic_curve_scalar_t scalar;
    ge_p3 point1;
    ge_p3 point2;
    ge_cached point3;
    ge_p1p1 point4;
    ge_p2 point5;
    if (ge_frombytes_vartime(&point1, reinterpret_cast<const unsigned char*>(&base)) != 0) {
      return false;
    }
    derivation_to_scalar(derivation, output_index, suffix, suffixLength, scalar);
    ge_scalarmult_base(&point2, reinterpret_cast<unsigned char*>(&scalar));
    ge_p3_to_cached(&point3, &point2);
    ge_add(&point4, &point1, &point3);
    ge_p1p1_to_p2(&point5, &point4);
    ge_tobytes(reinterpret_cast<unsigned char*>(&derived_key), &point5);
    return true;
  }

  bool crypto_ops::underive_public_key_and_get_scalar(const key_derivation_t &derivation, size_t output_index,
    const public_key_t &derived_key, public_key_t &base, elliptic_curve_scalar_t &hashed_derivation) {
    ge_p3 point1;
    ge_p3 point2;
    ge_cached point3;
    ge_p1p1 point4;
    ge_p2 point5;
    if (ge_frombytes_vartime(&point1, reinterpret_cast<const unsigned char*>(&derived_key)) != 0) {
      return false;
    }
    derivation_to_scalar(derivation, output_index, hashed_derivation);
    ge_scalarmult_base(&point2, reinterpret_cast<unsigned char*>(&hashed_derivation));
    ge_p3_to_cached(&point3, &point2);
    ge_sub(&point4, &point1, &point3);
    ge_p1p1_to_p2(&point5, &point4);
    ge_tobytes(reinterpret_cast<unsigned char*>(&base), &point5);
    return true;
  }

  void crypto_ops::derive_secret_key(const key_derivation_t &derivation, size_t output_index,
    const secret_key_t &base, secret_key_t &derived_key) {
    elliptic_curve_scalar_t scalar;
    assert(sc_check(reinterpret_cast<const unsigned char*>(&base)) == 0);
    derivation_to_scalar(derivation, output_index, scalar);
    sc_add(reinterpret_cast<unsigned char*>(&derived_key), reinterpret_cast<const unsigned char*>(&base), reinterpret_cast<unsigned char*>(&scalar));
  }

  void crypto_ops::derive_secret_key(const key_derivation_t &derivation, size_t output_index,
    const secret_key_t &base, const uint8_t* suffix, size_t suffixLength, secret_key_t &derived_key) {
    elliptic_curve_scalar_t scalar;
    assert(sc_check(reinterpret_cast<const unsigned char*>(&base)) == 0);
    derivation_to_scalar(derivation, output_index, suffix, suffixLength, scalar);
    sc_add(reinterpret_cast<unsigned char*>(&derived_key), reinterpret_cast<const unsigned char*>(&base), reinterpret_cast<unsigned char*>(&scalar));
  }


  bool crypto_ops::underive_public_key(const key_derivation_t &derivation, size_t output_index,
    const public_key_t &derived_key, public_key_t &base) {
    elliptic_curve_scalar_t scalar;
    ge_p3 point1;
    ge_p3 point2;
    ge_cached point3;
    ge_p1p1 point4;
    ge_p2 point5;
    if (ge_frombytes_vartime(&point1, reinterpret_cast<const unsigned char*>(&derived_key)) != 0) {
      return false;
    }
    derivation_to_scalar(derivation, output_index, scalar);
    ge_scalarmult_base(&point2, reinterpret_cast<unsigned char*>(&scalar));
    ge_p3_to_cached(&point3, &point2);
    ge_sub(&point4, &point1, &point3);
    ge_p1p1_to_p2(&point5, &point4);
    ge_tobytes(reinterpret_cast<unsigned char*>(&base), &point5);
    return true;
  }

  bool crypto_ops::underive_public_key(const key_derivation_t &derivation, size_t output_index,
    const public_key_t &derived_key, const uint8_t* suffix, size_t suffixLength, public_key_t &base) {
    elliptic_curve_scalar_t scalar;
    ge_p3 point1;
    ge_p3 point2;
    ge_cached point3;
    ge_p1p1 point4;
    ge_p2 point5;
    if (ge_frombytes_vartime(&point1, reinterpret_cast<const unsigned char*>(&derived_key)) != 0) {
      return false;
    }

    derivation_to_scalar(derivation, output_index, suffix, suffixLength, scalar);
    ge_scalarmult_base(&point2, reinterpret_cast<unsigned char*>(&scalar));
    ge_p3_to_cached(&point3, &point2);
    ge_sub(&point4, &point1, &point3);
    ge_p1p1_to_p2(&point5, &point4);
    ge_tobytes(reinterpret_cast<unsigned char*>(&base), &point5);
    return true;
  }


  struct s_comm {
    hash_t h;
    elliptic_curve_point_t key;
    elliptic_curve_point_t comm;
  };

  void crypto_ops::generate_signature(const hash_t &prefix_hash, const public_key_t &pub, const secret_key_t &sec, signature_t &sig) {
    lock_guard<mutex> lock(random_lock);
    ge_p3 tmp3;
    elliptic_curve_scalar_t k;
    s_comm buf;
#if !defined(NDEBUG)
    {
      ge_p3 t;
      public_key_t t2;
      assert(sc_check(reinterpret_cast<const unsigned char*>(&sec)) == 0);
      ge_scalarmult_base(&t, reinterpret_cast<const unsigned char*>(&sec));
      ge_p3_tobytes(reinterpret_cast<unsigned char*>(&t2), &t);
      assert(pub == t2);
    }
#endif
    buf.h = prefix_hash;
    buf.key = reinterpret_cast<const elliptic_curve_point_t&>(pub);
    random_scalar((uint8_t *)&k);
    ge_scalarmult_base(&tmp3, reinterpret_cast<unsigned char*>(&k));
    ge_p3_tobytes(reinterpret_cast<unsigned char*>(&buf.comm), &tmp3);
    hash_to_scalar((uint8_t *)&buf, sizeof(s_comm), (uint8_t *)&sig);
    sc_mulsub(reinterpret_cast<unsigned char*>(&sig) + 32, reinterpret_cast<unsigned char*>(&sig), reinterpret_cast<const unsigned char*>(&sec), reinterpret_cast<unsigned char*>(&k));
  }

  bool crypto_ops::check_signature(const hash_t &prefix_hash, const public_key_t &pub, const signature_t &sig) {
    ge_p2 tmp2;
    ge_p3 tmp3;
    elliptic_curve_scalar_t c;
    s_comm buf;
    assert(check_key((uint8_t*)&pub));
    buf.h = prefix_hash;
    buf.key = reinterpret_cast<const elliptic_curve_point_t&>(pub);
    if (ge_frombytes_vartime(&tmp3, reinterpret_cast<const unsigned char*>(&pub)) != 0) {
      abort();
    }
    if (sc_check(reinterpret_cast<const unsigned char*>(&sig)) != 0 || sc_check(reinterpret_cast<const unsigned char*>(&sig) + 32) != 0) {
      return false;
    }
    ge_double_scalarmult_base_vartime(&tmp2, reinterpret_cast<const unsigned char*>(&sig), &tmp3, reinterpret_cast<const unsigned char*>(&sig) + 32);
    ge_tobytes(reinterpret_cast<unsigned char*>(&buf.comm), &tmp2);
    hash_to_scalar((const uint8_t *)&buf, sizeof(s_comm), (uint8_t *)&c);
    sc_sub(reinterpret_cast<unsigned char*>(&c), reinterpret_cast<unsigned char*>(&c), reinterpret_cast<const unsigned char*>(&sig));
    return sc_isnonzero(reinterpret_cast<unsigned char*>(&c)) == 0;
  }

  static void hash_to_ec(const public_key_t &key, ge_p3 &res) {
    hash_t h;
    ge_p2 point;
    ge_p1p1 point2;
    cn_fast_hash(std::addressof(key), sizeof(public_key_t), h);
    ge_fromfe_frombytes_vartime(&point, reinterpret_cast<const unsigned char *>(&h));
    ge_mul8(&point2, &point);
    ge_p1p1_to_p3(&res, &point2);
  }

  void crypto_ops::hash_data_to_ec(const uint8_t* data, std::size_t len, public_key_t& key) {
    hash_t h;
    ge_p2 point;
    ge_p1p1 point2;
    cn_fast_hash(data, len, h);
    ge_fromfe_frombytes_vartime(&point, reinterpret_cast<const unsigned char *>(&h));
    ge_mul8(&point2, &point);
    ge_p1p1_to_p2(&point, &point2);
    ge_tobytes(reinterpret_cast<unsigned char*>(&key), &point);
  }
  
  void crypto_ops::generate_key_image(const public_key_t &pub, const secret_key_t &sec, key_image_t &image) {
    ge_p3 point;
    ge_p2 point2;
    assert(sc_check(reinterpret_cast<const unsigned char*>(&sec)) == 0);
    hash_to_ec(pub, point);
    ge_scalarmult(&point2, reinterpret_cast<const unsigned char*>(&sec), &point);
    ge_tobytes(reinterpret_cast<unsigned char*>(&image), &point2);
  }
  
  void crypto_ops::generate_incomplete_key_image(const public_key_t &pub, elliptic_curve_point_t &incomplete_key_image) {
    ge_p3 point;
    hash_to_ec(pub, point);
    ge_p3_tobytes(reinterpret_cast<unsigned char*>(&incomplete_key_image), &point);
  }

#ifdef _MSC_VER
#pragma warning(disable: 4200)
#endif

  struct rs_comm {
    hash_t h;
    struct {
      elliptic_curve_point_t a, b;
    } ab[];
  };

  static inline size_t rs_comm_size(size_t pubs_count) {
    rs_comm rs;
    return sizeof(rs_comm) + pubs_count * sizeof(rs.ab[0]);
  }

  void crypto_ops::generate_ring_signature(const hash_t &prefix_hash, const key_image_t &image,
    const public_key_t *const *pubs, size_t pubs_count,
    const secret_key_t &sec, size_t sec_index,
    signature_t *sig) {
    lock_guard<mutex> lock(random_lock);
    size_t i;
    ge_p3 image_unp;
    ge_dsmp image_pre;
    elliptic_curve_scalar_t sum, k, h;
    rs_comm *const buf = reinterpret_cast<rs_comm *>(alloca(rs_comm_size(pubs_count)));
    assert(sec_index < pubs_count);
#if !defined(NDEBUG)
    {
      ge_p3 t;
      public_key_t t2;
      key_image_t t3;
      assert(sc_check(reinterpret_cast<const unsigned char*>(&sec)) == 0);
      ge_scalarmult_base(&t, reinterpret_cast<const unsigned char*>(&sec));
      ge_p3_tobytes(reinterpret_cast<unsigned char*>(&t2), &t);
      assert(*pubs[sec_index] == t2);
      generate_key_image(*pubs[sec_index], sec, t3);
      assert(image == t3);
      for (i = 0; i < pubs_count; i++) {
        assert(check_key((uint8_t*)&(*pubs[i])));
      }
    }
#endif
    if (ge_frombytes_vartime(&image_unp, reinterpret_cast<const unsigned char*>(&image)) != 0) {
      abort();
    }
    ge_dsm_precomp(image_pre, &image_unp);
    sc_0(reinterpret_cast<unsigned char*>(&sum));
    buf->h = prefix_hash;
    for (i = 0; i < pubs_count; i++) {
      ge_p2 tmp2;
      ge_p3 tmp3;
      if (i == sec_index) {
        random_scalar((uint8_t *)&k);
        ge_scalarmult_base(&tmp3, reinterpret_cast<unsigned char*>(&k));
        ge_p3_tobytes(reinterpret_cast<unsigned char*>(&buf->ab[i].a), &tmp3);
        hash_to_ec(*pubs[i], tmp3);
        ge_scalarmult(&tmp2, reinterpret_cast<unsigned char*>(&k), &tmp3);
        ge_tobytes(reinterpret_cast<unsigned char*>(&buf->ab[i].b), &tmp2);
      } else {
        random_scalar((uint8_t *)&(sig[i]));
        random_scalar((uint8_t *)(reinterpret_cast<unsigned char*>(&sig[i]) + 32));
        if (ge_frombytes_vartime(&tmp3, reinterpret_cast<const unsigned char*>(&*pubs[i])) != 0) {
          abort();
        }
        ge_double_scalarmult_base_vartime(&tmp2, reinterpret_cast<unsigned char*>(&sig[i]), &tmp3, reinterpret_cast<unsigned char*>(&sig[i]) + 32);
        ge_tobytes(reinterpret_cast<unsigned char*>(&buf->ab[i].a), &tmp2);
        hash_to_ec(*pubs[i], tmp3);
        ge_double_scalarmult_precomp_vartime(&tmp2, reinterpret_cast<unsigned char*>(&sig[i]) + 32, &tmp3, reinterpret_cast<unsigned char*>(&sig[i]), image_pre);
        ge_tobytes(reinterpret_cast<unsigned char*>(&buf->ab[i].b), &tmp2);
        sc_add(reinterpret_cast<unsigned char*>(&sum), reinterpret_cast<unsigned char*>(&sum), reinterpret_cast<unsigned char*>(&sig[i]));
      }
    }
    hash_to_scalar((uint8_t *)buf, rs_comm_size(pubs_count), (uint8_t *)&h);
    sc_sub(reinterpret_cast<unsigned char*>(&sig[sec_index]), reinterpret_cast<unsigned char*>(&h), reinterpret_cast<unsigned char*>(&sum));
    sc_mulsub(reinterpret_cast<unsigned char*>(&sig[sec_index]) + 32, reinterpret_cast<unsigned char*>(&sig[sec_index]), reinterpret_cast<const unsigned char*>(&sec), reinterpret_cast<unsigned char*>(&k));
  }

  bool crypto_ops::check_ring_signature(const hash_t &prefix_hash, const key_image_t &image,
    const public_key_t *const *pubs, size_t pubs_count,
    const signature_t *sig) {
    size_t i;
    ge_p3 image_unp;
    ge_dsmp image_pre;
    elliptic_curve_scalar_t sum, h;
    rs_comm *const buf = reinterpret_cast<rs_comm *>(alloca(rs_comm_size(pubs_count)));
#if !defined(NDEBUG)
    for (i = 0; i < pubs_count; i++) {
      assert(check_key((uint8_t*)&(*pubs[i])));
    }
#endif
    if (ge_frombytes_vartime(&image_unp, reinterpret_cast<const unsigned char*>(&image)) != 0) {
      return false;
    }
    ge_dsm_precomp(image_pre, &image_unp);
    sc_0(reinterpret_cast<unsigned char*>(&sum));
    buf->h = prefix_hash;
    for (i = 0; i < pubs_count; i++) {
      ge_p2 tmp2;
      ge_p3 tmp3;
      if (sc_check(reinterpret_cast<const unsigned char*>(&sig[i])) != 0 || sc_check(reinterpret_cast<const unsigned char*>(&sig[i]) + 32) != 0) {
        return false;
      }
      if (ge_frombytes_vartime(&tmp3, reinterpret_cast<const unsigned char*>(&*pubs[i])) != 0) {
        abort();
      }
      ge_double_scalarmult_base_vartime(&tmp2, reinterpret_cast<const unsigned char*>(&sig[i]), &tmp3, reinterpret_cast<const unsigned char*>(&sig[i]) + 32);
      ge_tobytes(reinterpret_cast<unsigned char*>(&buf->ab[i].a), &tmp2);
      hash_to_ec(*pubs[i], tmp3);
      ge_double_scalarmult_precomp_vartime(&tmp2, reinterpret_cast<const unsigned char*>(&sig[i]) + 32, &tmp3, reinterpret_cast<const unsigned char*>(&sig[i]), image_pre);
      ge_tobytes(reinterpret_cast<unsigned char*>(&buf->ab[i].b), &tmp2);
      sc_add(reinterpret_cast<unsigned char*>(&sum), reinterpret_cast<unsigned char*>(&sum), reinterpret_cast<const unsigned char*>(&sig[i]));
    }
    hash_to_scalar((uint8_t *)buf, rs_comm_size(pubs_count), (uint8_t *)&h);
    sc_sub(reinterpret_cast<unsigned char*>(&h), reinterpret_cast<unsigned char*>(&h), reinterpret_cast<unsigned char*>(&sum));
    return sc_isnonzero(reinterpret_cast<unsigned char*>(&h)) == 0;
  }
}
