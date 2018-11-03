// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "CommonTypes.h"
#include "IStreamSerializable.h"
#include "serialization/ISerializer.h"
#include <vector>
#include <map>

namespace cryptonote {

class SynchronizationState : public IStreamSerializable {
public:

  struct CheckResult {
    bool detachRequired;
    uint32_t detachHeight;
    bool hasNewBlocks;
    uint32_t newBlockHeight;
  };

  typedef std::vector<crypto::hash_t> ShortHistory;

  explicit SynchronizationState(const crypto::hash_t& genesisBlockHash) {
    m_blockchain.push_back(genesisBlockHash);
  }
  virtual ~SynchronizationState() {};

  ShortHistory getShortHistory(uint32_t localHeight) const;
  CheckResult checkInterval(const BlockchainInterval& interval) const;

  void detach(uint32_t height);
  void addBlocks(const crypto::hash_t* blockHashes, uint32_t height, uint32_t count);
  uint32_t getHeight() const;
  const std::vector<crypto::hash_t>& getKnownBlockHashes() const;

  // IStreamSerializable
  virtual void save(std::ostream& os) override;
  virtual void load(std::istream& in) override;

  // serialization
  cryptonote::ISerializer& serialize(cryptonote::ISerializer& s, const std::string& name);

private:

  std::vector<crypto::hash_t> m_blockchain;
};

}
