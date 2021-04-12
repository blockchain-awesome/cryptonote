#pragma once

#include <string>
#include <unordered_map>
#include <map>

#include "cryptonote/crypto/hash.h"
#include "cryptonote/core/key.h"
#include "stream/map.hpp"

namespace cryptonote
{

  class ISerializer;

  class TimestampBlocksIndex
  {
  public:
    TimestampBlocksIndex() = default;

    bool add(uint64_t timestamp, const hash_t &hash);
    bool remove(uint64_t timestamp, const hash_t &hash);
    bool find(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t hashesNumberLimit, std::vector<hash_t> &hashes, uint32_t &hashesNumberWithinTimestamps);
    void clear();

    std::multimap<uint64_t, hash_t> index;
  };
  
  Reader &operator>>(Reader &i, TimestampBlocksIndex &v);

  Writer &operator<<(Writer &o, const TimestampBlocksIndex &v);

} // namespace cryptonote