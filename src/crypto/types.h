
#pragma once

#include <stdint.h>

#define CHACHA_KEY_SIZE 32
#define CHACHA_IV_SIZE 8

typedef struct
{
  uint8_t data[32];
} hash_t;

typedef struct
{
  uint8_t data[32];
} public_key_t;

typedef struct
{
  uint8_t data[32];
} secret_key_t;

typedef struct
{
  uint8_t data[32];
} key_derivation_t;

typedef struct
{
  uint8_t data[32];
} key_image_t;

typedef struct
{
  uint8_t data[64];
} signature_t;


#pragma pack(push, 1)
struct chacha_key_t
{
  uint8_t data[CHACHA_KEY_SIZE];
};

struct chacha_iv_t
{
  uint8_t data[CHACHA_IV_SIZE];
};
#pragma pack(pop)

typedef struct
{
  uint8_t data[32];
} elliptic_curve_point_t;

typedef struct
{
  uint8_t data[32];
} elliptic_curve_scalar_t;

// typedef struct
// {
//   hash_t h;
//   struct
//   {
//     elliptic_curve_point_t a, b;
//   } ab[];
// } rs_comm_t;
