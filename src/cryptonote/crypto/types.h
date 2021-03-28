#pragma once

#include <stdint.h>
#include <stddef.h>

#define CHACHA_KEY_SIZE 32
#define CHACHA_IV_SIZE 8

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