
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
