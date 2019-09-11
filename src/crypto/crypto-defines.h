
#pragma once

#include <stdint.h>

typedef struct
{
  uint8_t data[32];
} elliptic_curve_point_t;

typedef struct
{
  uint8_t data[32];
} elliptic_curve_scalar_t;

#define VARINT_WRITE(DEST, INT)            \
  while ((INT) >= 0x80)                      \
  {                                        \
    *(DEST)++ = ((char)(INT)&0x7f) | 0x80; \
    (INT) >>= 7;                           \
  }                                        \
  *DEST++ = (char)(INT);

void random_scalar(uint8_t *res);
void hash_to_scalar(const uint8_t *scalar, size_t length, uint8_t *hash);
void generate_keys(uint8_t *public_key, uint8_t *secret_key);

int check_key(const uint8_t *public_key);
int secret_key_to_public_key(const uint8_t *secret_key, uint8_t *public_key);
int generate_key_derivation(const uint8_t *public_key, const uint8_t *secret_key, uint8_t *key_derivation);

int derive_public_key(const uint8_t *key_derivation, size_t, const uint8_t *base, uint8_t *derived_key);

//hack for pg
int underive_public_key_and_get_scalar(const uint8_t *key_derivation, size_t, const uint8_t *, uint8_t *, uint8_t *);

void generate_incomplete_key_image(const uint8_t *public_key, uint8_t *);

//
void derive_secret_key(const uint8_t *key_derivation, size_t, const uint8_t *base, uint8_t *derived_key);

// void derive_secret_key(const uint8_t *key_derivation, size_t, const uint8_t *base, const uint8_t*, size_t, uint8_t *derived_key);

// int underive_public_key(const uint8_t *key_derivation, size_t, const uint8_t *public_key, uint8_t *public_key);

// int underive_public_key(const uint8_t *key_derivation, size_t, const uint8_t *public_key, const uint8_t*, size_t, uint8_t *public_key);

void generate_signature(const uint8_t *hash, const uint8_t *public_key, const uint8_t *secret_key, uint8_t *signature);

int check_signature(const uint8_t *hash, const uint8_t *public_key, const uint8_t *signature);

void generate_key_image(const uint8_t *public_key, const uint8_t *secret_key, uint8_t *key_image);

void hash_data_to_ec(const uint8_t *, size_t, uint8_t *public_key);

void generate_ring_signature(const uint8_t *hash, const uint8_t *key_image,
                             const uint8_t *const *public_key, size_t, const uint8_t *secret_key, size_t, uint8_t *signature);

int check_ring_signature(const uint8_t *hash, const uint8_t *key_image,
                         const uint8_t *const *public_key, size_t, const uint8_t *signature);
