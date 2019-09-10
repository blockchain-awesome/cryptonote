
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
  return 0;
}

int derive_public_key(const uint8_t *key_derivation, size_t size, const uint8_t *base, uint8_t *derived_key)
{
  return 0;
}

//hack for pg
int underive_public_key_and_get_scalar(const uint8_t *key_derivation, size_t size, const uint8_t *public_key, uint8_t *public_key1, uint8_t *scalar)
{
  return 0;
}

void generate_incomplete_key_image(const uint8_t *public_key, uint8_t *key_image) {}

//
void derive_secret_key(const uint8_t *key_derivation, size_t size, const uint8_t *base, uint8_t *derived_key) {}

// void derive_secret_key(const uint8_t *key_derivation, size_t size, const uint8_t *base, const uint8_t*, size_t, uint8_t *derived_key){}

// int underive_public_key(const uint8_t *key_derivation, size_t, const uint8_t *public_key, uint8_t *public_key){}

// int underive_public_key(const uint8_t *key_derivation, size_t, const uint8_t *public_key, const uint8_t*, size_t, uint8_t *public_key){}

void generate_signature(const uint8_t *hash, const uint8_t *public_key, const uint8_t *secret_key, uint8_t *signature) {}

int check_signature(const uint8_t *hash, const uint8_t *public_key, const uint8_t *signature)
{
  return 0;
}

void generate_key_image(const uint8_t *public_key, const uint8_t *secret_key, uint8_t *key_image) {}

void hash_data_to_ec(const uint8_t *scalar, size_t size, uint8_t *public_key) {}

void generate_ring_signature(const uint8_t *hash, const uint8_t *key_image,
                             const uint8_t *const *public_key, size_t public_size, const uint8_t *secret_key, size_t secret_size, uint8_t *signature) {}

int check_ring_signature(const uint8_t *hash, const uint8_t *key_image,
                         const uint8_t *const *public_key, size_t size, const uint8_t *signature)
{
  return 0;
}