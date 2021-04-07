#pragma once

#include <string>
#include <unordered_map>
#include <map>

#include "cryptonote/crypto/hash.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/blockchain/serializer/map.h"

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

    void serialize(ISerializer &s);

    template <class Archive>
    void serialize(Archive &archive, unsigned int version)
    {
      archive &index;
    }

    std::multimap<uint64_t, hash_t> index;
  };
  Reader &operator>>(Reader &i, TimestampBlocksIndex &v);

  Writer &operator<<(Writer &o, const TimestampBlocksIndex &v);

} // namespace cryptonote