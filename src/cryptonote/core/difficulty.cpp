// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "crypto/util.h"
#include "crypto/types.h"
#include "difficulty.h"

extern "C"
{

#if defined(__SIZEOF_INT128__)

  static inline void mul(uint64_t a, uint64_t b, uint64_t &low, uint64_t &high)
  {
    typedef unsigned __int128 uint128_t;
    uint128_t res = (uint128_t)a * (uint128_t)b;
    low = (uint64_t)res;
    high = (uint64_t)(res >> 64);
  }

#else

  static inline void mul(uint64_t a, uint64_t b, uint64_t &low, uint64_t &high)
  {
    low = mul128(a, b, &high);
  }

#endif

  static inline bool cadd(uint64_t a, uint64_t b)
  {
    return a + b < a;
  }

  static inline bool cadc(uint64_t a, uint64_t b, bool c)
  {
    return a + b < a || (c && a + b == (uint64_t)-1);
  }

  bool check_hash(const hash_t *hash, uint64_t difficulty)
  {
    uint64_t low, high, top, cur;
    // First check the highest word, this will most likely fail for a random hash.
    mul(swap64le(((const uint64_t *)hash)[3]), difficulty, top, high);
    if (high != 0)
    {
      return false;
    }
    mul(swap64le(((const uint64_t *)hash)[0]), difficulty, low, cur);
    mul(swap64le(((const uint64_t *)hash)[1]), difficulty, low, high);
    bool carry = cadd(cur, low);
    cur = high;
    mul(swap64le(((const uint64_t *)hash)[2]), difficulty, low, high);
    carry = cadc(cur, low, carry);
    carry = cadc(high, top, carry);
    return !carry;
  }

  int compare(const void *a, const void *b)
  {
    return (*(int *)a - *(int *)b);
  }

  uint64_t next_difficulty(uint64_t *timestamps,
                           uint16_t timestamps_length,
                           uint64_t *cumulativeDifficulties,
                           uint16_t difficulties_length,
                           uint64_t *conf)
  {
    difficulty_config_t *config = (difficulty_config_t *)conf;

    assert(config->window >= 2);

    if (timestamps_length > config->window)
    {
      timestamps_length = config->window;
      difficulties_length = config->window;
    }

    size_t length = timestamps_length;
    assert(length == difficulties_length);
    assert(length <= config->window);
    if (length <= 1)
    {
      return 1;
    }

    qsort(timestamps, timestamps_length, sizeof(uint64_t), compare);

    size_t cutBegin, cutEnd;
    assert(2 * config->cut <= config->window - 2);

    if (length <= config->window - 2 * config->cut)
    {
      cutBegin = 0;
      cutEnd = length;
    }
    else
    {
      cutBegin = (length - (config->window - 2 * config->cut) + 1) / 2;
      cutEnd = cutBegin + (config->window - 2 * config->cut);
    }
    assert(/*cut_begin >= 0 &&*/ cutBegin + 2 <= cutEnd && cutEnd <= length);
    uint64_t timeSpan = timestamps[cutEnd - 1] - timestamps[cutBegin];
    if (timeSpan == 0)
    {
      timeSpan = 1;
    }

    uint64_t totalWork = cumulativeDifficulties[cutEnd - 1] - cumulativeDifficulties[cutBegin];
    assert(totalWork > 0);

    uint64_t low, high;
    low = mul128(totalWork, config->target, &high);
    if (high != 0 || low + timeSpan - 1 < low)
    {
      return 0;
    }

    return (low + timeSpan - 1) / timeSpan;
  }
}
