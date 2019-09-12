
#include <stdint.h>
#include <memory.h>
#include "crypto-ops.h"
#include "crypto-defines.h"
#include "random.h"
#include "hash-ops.h"

void random_scalar(uint8_t *res)
{
  unsigned char tmp[64];
  generate_random_bytes_not_thread_safe(64, tmp);
  sc_reduce(tmp);
  memcpy(res, tmp, 32);
}

void hash_to_scalar(const uint8_t *scalar, size_t length, uint8_t *hash)
{
  cn_fast_hash(scalar, length, (char *)hash);
  sc_reduce32(hash);
}

void generate_keys(uint8_t *public_key, uint8_t *secret_key)
{
  ge_p3 point;
  random_scalar(secret_key);
  ge_scalarmult_base(&point, secret_key);
  ge_p3_tobytes(public_key, &point);
}

int check_key(const uint8_t *public_key)
{
  ge_p3 point;
  return ge_frombytes_vartime(&point, public_key) == 0;
}

int secret_key_to_public_key(const uint8_t *secret_key, uint8_t *public_key)
{
  ge_p3 point;
  if (sc_check(secret_key) != 0)
  {
    return 0;
  }
  ge_scalarmult_base(&point, secret_key);
  ge_p3_tobytes(public_key, &point);
  return 1;
}

int generate_key_derivation(const uint8_t *public_key, const uint8_t *secret_key, uint8_t *key_derivation)
{
  ge_p3 point;
  ge_p2 point2;
  ge_p1p1 point3;
  assert(sc_check(secret_key) == 0);
  if (ge_frombytes_vartime(&point, public_key) != 0)
  {
    return 0;
  }
  ge_scalarmult(&point2, secret_key, &point);
  ge_mul8(&point3, &point2);
  ge_p1p1_to_p2(&point2, &point3);
  ge_tobytes(key_derivation, &point2);
  return 1;
}
