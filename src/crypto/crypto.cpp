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

#include "crypto.h"
#include "hash.h"

namespace crypto
{

using std::abort;
using std::int32_t;
using std::lock_guard;
using std::mutex;

extern "C"
{
#include "crypto-ops.h"
#include "random.h"
#include "crypto-defines.h"
}

mutex random_lock;

static void hash_to_ec(const uint8_t *key, ge_p3 &res)
{
  hash_t h;
  ge_p2 point;
  ge_p1p1 point2;
  cn_fast_hash(key, 32, (char *)&h);
  ge_fromfe_frombytes_vartime(&point, (const uint8_t *)(&h));
  ge_mul8(&point2, &point);
  ge_p1p1_to_p3(&res, &point2);
}

void generate_key_image(const uint8_t *pub, const uint8_t *sec, uint8_t *image)
{
  ge_p3 point;
  ge_p2 point2;
  assert(sc_check(sec) == 0);
  hash_to_ec(pub, point);
  ge_scalarmult(&point2, sec, &point);
  ge_tobytes(image, &point2);
}

void generate_incomplete_key_image(const uint8_t *pub, uint8_t *incomplete_key_image)
{
  ge_p3 point;
  hash_to_ec(pub, point);
  ge_p3_tobytes(incomplete_key_image, &point);
}

#ifdef _MSC_VER
#pragma warning(disable : 4200)
#endif

struct rs_comm
{
  hash_t h;
  struct
  {
    elliptic_curve_point_t a, b;
  } ab[];
};

static inline size_t rs_comm_size(size_t pubs_count)
{
  rs_comm rs;
  return sizeof(rs_comm) + pubs_count * sizeof(rs.ab[0]);
}

void generate_ring_signature(const uint8_t *prefix_hash, const uint8_t *image,
                             const public_key_t *const *pubs, size_t pubs_count,
                             const uint8_t *sec, size_t sec_index,
                             uint8_t *sig)
{
  // lock_guard<mutex> lock(random_lock);
  size_t i;
  ge_p3 image_unp;
  ge_dsmp image_pre;
  elliptic_curve_scalar_t sum, k, h;
  rs_comm *const buf = (rs_comm *)(alloca(rs_comm_size(pubs_count)));
  assert(sec_index < pubs_count);
#if !defined(NDEBUG)
  {
    ge_p3 t;
    public_key_t t2;
    key_image_t t3;
    assert(sc_check(sec) == 0);
    ge_scalarmult_base(&t, sec);
    ge_p3_tobytes((uint8_t *)(&t2), &t);
    assert(*pubs[sec_index] == t2);
    generate_key_image((const uint8_t *)&(*pubs[sec_index]), sec, (uint8_t *)&t3);
    assert(*(key_image_t *)image == t3);
    for (i = 0; i < pubs_count; i++)
    {
      assert(check_key((uint8_t *)&(*pubs[i])));
    }
  }
#endif
  if (ge_frombytes_vartime(&image_unp, image) != 0)
  {
    abort();
  }
  ge_dsm_precomp(image_pre, &image_unp);
  sc_0((uint8_t *)(&sum));
  buf->h = *(const hash_t *)prefix_hash;

  for (i = 0; i < pubs_count; i++)
  {
    ge_p2 tmp2;
    ge_p3 tmp3;
    if (i == sec_index)
    {
      random_scalar((uint8_t *)&k);
      ge_scalarmult_base(&tmp3, (uint8_t *)(&k));
      ge_p3_tobytes((uint8_t *)(&buf->ab[i].a), &tmp3);
      hash_to_ec((const uint8_t *)pubs[i], tmp3);
      ge_scalarmult(&tmp2, (uint8_t *)(&k), &tmp3);
      ge_tobytes((uint8_t *)(&buf->ab[i].b), &tmp2);
    }
    else
    {
      random_scalar((uint8_t *)&(sig[i * 64]));
      random_scalar((uint8_t *)((uint8_t *)(&sig[i * 64]) + 32));
      if (ge_frombytes_vartime(&tmp3, (const uint8_t *)(&*pubs[i])) != 0)
      {
        abort();
      }
      ge_double_scalarmult_base_vartime(&tmp2, (uint8_t *)(&sig[i * 64]), &tmp3, (uint8_t *)(&sig[i * 64]) + 32);
      ge_tobytes((uint8_t *)(&buf->ab[i].a), &tmp2);
      hash_to_ec((const uint8_t *)pubs[i], tmp3);
      ge_double_scalarmult_precomp_vartime(&tmp2, (uint8_t *)(&sig[i*64]) + 32, &tmp3, (uint8_t *)(&sig[i * 64]), image_pre);
      ge_tobytes((uint8_t *)(&buf->ab[i].b), &tmp2);
      sc_add((uint8_t *)(&sum), (uint8_t *)(&sum), (uint8_t *)(&sig[i*64]));
    }
  }
  hash_to_scalar((uint8_t *)buf, rs_comm_size(pubs_count), (uint8_t *)&h);
  sc_sub((uint8_t *)(&sig[sec_index * 64]), (uint8_t *)(&h), (uint8_t *)(&sum));
  sc_mulsub((uint8_t *)(&sig[sec_index * 64]) + 32, (uint8_t *)(&sig[sec_index * 64]), sec, (uint8_t *)(&k));
}

bool check_ring_signature(const uint8_t *prefix_hash, const uint8_t *image,
                          const uint8_t *const *pubs, size_t pubs_count,
                          const uint8_t *sig)
{
  size_t i;
  ge_p3 image_unp;
  ge_dsmp image_pre;
  elliptic_curve_scalar_t sum, h;
  rs_comm *const buf = (rs_comm *)(alloca(rs_comm_size(pubs_count)));
#if !defined(NDEBUG)
  for (i = 0; i < pubs_count; i++)
  {
    assert(check_key((uint8_t *)(pubs + i * 32)));
  }
#endif
  if (ge_frombytes_vartime(&image_unp, image) != 0)
  {
    return false;
  }
  ge_dsm_precomp(image_pre, &image_unp);
  sc_0((uint8_t *)(&sum));
  buf->h = *(const hash_t *)prefix_hash;
  for (i = 0; i < pubs_count; i++)
  {
    ge_p2 tmp2;
    ge_p3 tmp3;
    if (sc_check(sig + i * 64) != 0 || sc_check(sig + i * 64 + 32) != 0)
    {
      return false;
    }
    if (ge_frombytes_vartime(&tmp3, sig + i * 64) != 0)
    {
      abort();
    }
    ge_double_scalarmult_base_vartime(&tmp2, sig + i * 64, &tmp3, sig + i * 64 + 32);
    ge_tobytes((uint8_t *)(&buf->ab[i].a), &tmp2);
    hash_to_ec(*(pubs + i * 32), tmp3);
    ge_double_scalarmult_precomp_vartime(&tmp2, sig + i * 64 + 32, &tmp3, sig + i * 64, image_pre);
    ge_tobytes((uint8_t *)(&buf->ab[i].b), &tmp2);
    sc_add((uint8_t *)(&sum), (const uint8_t *)(&sum), sig + i * 64);
  }
  hash_to_scalar((uint8_t *)buf, rs_comm_size(pubs_count), (uint8_t *)&h);
  sc_sub((uint8_t *)(&h), (uint8_t *)(&h), (uint8_t *)(&sum));
  return sc_isnonzero((uint8_t *)(&h)) == 0;
}
} // namespace crypto
