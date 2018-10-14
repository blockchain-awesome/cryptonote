#pragma once

#include <unordered_map>
#include "crypto/hash.h"
#include "cryptonote/core/CryptoNoteBasic.h"

namespace cryptonote
{

class ISerializer;

class OrphanBlocksIndex {
public:
  OrphanBlocksIndex() = default;

  bool add(const Block& block);
  bool remove(const Block& block);
  bool find(uint32_t height, std::vector<crypto::Hash>& blockHashes);
  void clear();
private:
  std::unordered_multimap<uint32_t, crypto::Hash> index;
};

} // namespace cryptonote
