#pragma once

#include <stdint.h>
#include <stddef.h>

#define CHACHA_KEY_SIZE 32
#define CHACHA_IV_SIZE 8

#if defined(__cplusplus)
#include <memory.h>
#include <string>

#include "hash.h"
#include "crypto/types.h"

namespace crypto
{
  extern "C"
  {
#endif
    void chacha8(const void *data, size_t length, const uint8_t *key, const uint8_t *iv, char *cipher);
#if defined(__cplusplus)
  }

  static_assert(sizeof(chacha_key_t) == CHACHA_KEY_SIZE && sizeof(chacha_iv_t) == CHACHA_IV_SIZE, "Invalid structure size");

  inline void chacha8(const void *data, size_t length, const chacha_key_t &key, const chacha_iv_t &iv, char *cipher)
  {
    chacha8(data, length, reinterpret_cast<const uint8_t *>(&key), reinterpret_cast<const uint8_t *>(&iv), cipher);
  }

  inline void generate_chacha_key(const std::string &password, chacha_key_t &key)
  {
    static_assert(sizeof(chacha_key_t) <= sizeof(hash_t), "Size of hash must be at least that of chacha_key_t");
    hash_t pwd_hash;
    cn_slow_hash(password.data(), password.size(), (char *)&pwd_hash, 0, 0);
    memcpy(key.data, &pwd_hash, sizeof(key.data));
    // memset(&pwd_hash, 0, sizeof(pwd_hash));
  }
}

#endif
