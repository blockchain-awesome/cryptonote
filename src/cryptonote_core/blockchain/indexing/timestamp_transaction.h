#pragma once

#include <map>

#include "crypto/hash.h"

#include "CryptoNoteBasic.h"

namespace cryptonote
{

class ISerializer;

class TimestampTransactionsIndex
{
public:
  TimestampTransactionsIndex() = default;

  bool add(uint64_t timestamp, const crypto::Hash &hash);
  bool remove(uint64_t timestamp, const crypto::Hash &hash);
  bool find(uint64_t timestampBegin, uint64_t timestampEnd, uint64_t hashesNumberLimit, std::vector<crypto::Hash> &hashes, uint64_t &hashesNumberWithinTimestamps);
  void clear();

  void serialize(ISerializer &s);

  template <class Archive>
  void serialize(Archive &archive, unsigned int version)
  {
    archive &index;
  }

private:
  std::multimap<uint64_t, crypto::Hash> index;
};
} // namespace cryptonote