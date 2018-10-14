#include "orphan_block.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"

namespace cryptonote
{


bool OrphanBlocksIndex::add(const Block& block) {
  crypto::Hash blockHash = get_block_hash(block);
  uint32_t blockHeight = boost::get<BaseInput>(block.baseTransaction.inputs.front()).blockIndex;
  index.emplace(blockHeight, blockHash);
  return true;
}

bool OrphanBlocksIndex::remove(const Block& block) {
  crypto::Hash blockHash = get_block_hash(block);
  uint32_t blockHeight = boost::get<BaseInput>(block.baseTransaction.inputs.front()).blockIndex;
  auto range = index.equal_range(blockHeight);
  for (auto iter = range.first; iter != range.second; ++iter) {
    if (iter->second == blockHash) {
      index.erase(iter);
      return true;
    }
  }

  return false;
}

bool OrphanBlocksIndex::find(uint32_t height, std::vector<crypto::Hash>& blockHashes) {
  if (height > std::numeric_limits<uint32_t>::max()) {
    return false;
  }
  bool found = false;
  auto range = index.equal_range(height);
  for (auto iter = range.first; iter != range.second; ++iter) {
    found = true;
    blockHashes.emplace_back(iter->second);
  }
  return found;
}

void OrphanBlocksIndex::clear() {
  index.clear();
}

} // namespace cryptonote
