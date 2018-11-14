// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "block_index.h"

#include <boost/utility/value_init.hpp>

#include "cryptonote/core/blockchain/serializer/basics.h"
#include "serialization/SerializationOverloads.h"

namespace cryptonote {
  crypto::hash_t BlockIndex::getBlockId(uint32_t height) const {
    assert(height < m_container.size());

    return m_container[static_cast<size_t>(height)];
  }

  std::vector<crypto::hash_t> BlockIndex::getBlockIds(uint32_t startBlockIndex, uint32_t maxCount) const {
    std::vector<crypto::hash_t> result;
    if (startBlockIndex >= m_container.size()) {
      return result;
    }

    size_t count = std::min(static_cast<size_t>(maxCount), m_container.size() - static_cast<size_t>(startBlockIndex));
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      result.push_back(m_container[startBlockIndex + i]);
    }

    return result;
  }

  bool BlockIndex::findSupplement(const std::vector<crypto::hash_t>& ids, uint32_t& offset) const {
    for (const auto& id : ids) {
      if (getBlockHeight(id, offset)) {
        return true;
      }
    }

    return false;
  }

  std::vector<crypto::hash_t> BlockIndex::buildSparseChain(const crypto::hash_t& startBlockId) const {
    assert(m_index.count(startBlockId) > 0);

    uint32_t startBlockHeight = 0;
    getBlockHeight(startBlockId, startBlockHeight);

    std::vector<crypto::hash_t> result;
    size_t sparseChainEnd = static_cast<size_t>(startBlockHeight + 1);
    for (size_t i = 1; i <= sparseChainEnd; i *= 2) {
      result.emplace_back(m_container[sparseChainEnd - i]);
    }

    if (result.back() != m_container[0]) {
      result.emplace_back(m_container[0]);
    }

    return result;
  }

  crypto::hash_t BlockIndex::getTailId() const {
    assert(!m_container.empty());
    return m_container.back();
  }

  void BlockIndex::serialize(ISerializer& s) {
    if (s.type() == ISerializer::INPUT) {
      readSequence<crypto::hash_t>(std::back_inserter(m_container), "index", s);
    } else {
      writeSequence<crypto::hash_t>(m_container.begin(), m_container.end(), "index", s);
    }
  }
}
