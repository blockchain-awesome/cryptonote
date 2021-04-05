// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DIFFICULTY_H_
#define DIFFICULTY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "types.h"
  extern void mul(uint64_t a, uint64_t b, uint64_t *low, uint64_t *high);
  extern bool check_hash(const hash_t *hash, difficulty_t difficulty);
  extern uint64_t next_difficulty(uint64_t *timestamps,
                                  uint16_t timestamps_length,
                                  uint64_t *cumulativeDifficulties,
                                  uint64_t *config);

#ifdef __cplusplus
} // extern "C"
#endif

#endif