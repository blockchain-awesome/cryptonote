// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stddef.h>

#include <crypto.h>
#include "generic-ops.h"

namespace crypto {

  extern "C" {
#include "hash-ops.h"
  }

  /*
    Cryptonight hash functions
  */

  inline void cn_fast_hash(const void *data, size_t length, hash_t &hash) {
    cn_fast_hash(data, length, reinterpret_cast<char *>(&hash));
  }

  inline hash_t cn_fast_hash(const void *data, size_t length) {
    hash_t h;
    cn_fast_hash(data, length, reinterpret_cast<char *>(&h));
    return h;
  }

  inline void cn_slow_hash(const void *data, std::size_t length, hash_t &hash, int variant = 0) {
    cn_slow_hash(data, length, reinterpret_cast<char *>(&hash), variant, 0/*prehashed*/);
  }

  inline void tree_hash(const hash_t *hashes, size_t count, hash_t &root_hash) {
    tree_hash(reinterpret_cast<const char (*)[HASH_SIZE]>(hashes), count, reinterpret_cast<char *>(&root_hash));
  }

  inline void tree_branch(const hash_t *hashes, size_t count, hash_t *branch) {
    tree_branch(reinterpret_cast<const char (*)[HASH_SIZE]>(hashes), count, reinterpret_cast<char (*)[HASH_SIZE]>(branch));
  }

  inline void tree_hash_from_branch(const hash_t *branch, size_t depth, const hash_t &leaf, const void *path, hash_t &root_hash) {
    tree_hash_from_branch(reinterpret_cast<const char (*)[HASH_SIZE]>(branch), depth, reinterpret_cast<const char *>(&leaf), path, reinterpret_cast<char *>(&root_hash));
  }

}

CRYPTO_MAKE_HASHABLE(hash_t)
