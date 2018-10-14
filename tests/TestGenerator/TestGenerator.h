// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <list>
#include <vector>
#include <unordered_map>

#include "crypto/hash.h"
#include "cryptonote_core/CryptoNoteBasic.h"
#include "cryptonote_core/CryptoNoteBasicImpl.h"
#include "cryptonote_core/CryptoNoteFormatUtils.h"
#include "cryptonote_core/Currency.h"
#include "cryptonote_core/Difficulty.h"


class test_generator
{
public:
  struct BlockInfo {
    BlockInfo()
      : previousBlockHash()
      , alreadyGeneratedCoins(0)
      , blockSize(0) {
    }

    BlockInfo(crypto::Hash aPrevId, uint64_t anAlreadyGeneratedCoins, size_t aBlockSize)
      : previousBlockHash(aPrevId)
      , alreadyGeneratedCoins(anAlreadyGeneratedCoins)
      , blockSize(aBlockSize) {
    }

    crypto::Hash previousBlockHash;
    uint64_t alreadyGeneratedCoins;
    size_t blockSize;
  };

  enum BlockFields {
    bf_none      = 0,
    bf_major_ver = 1 << 0,
    bf_minor_ver = 1 << 1,
    bf_timestamp = 1 << 2,
    bf_prev_id   = 1 << 3,
    bf_miner_tx  = 1 << 4,
    bf_tx_hashes = 1 << 5,
    bf_diffic    = 1 << 6
  };

  test_generator(const cryptonote::Currency& currency, uint8_t majorVersion = cryptonote::BLOCK_MAJOR_VERSION_1,
                 uint8_t minorVersion = cryptonote::BLOCK_MINOR_VERSION_0)
    : m_currency(currency), defaultMajorVersion(majorVersion), defaultMinorVersion(minorVersion) {
  }


  uint8_t defaultMajorVersion;
  uint8_t defaultMinorVersion;

  const cryptonote::Currency& currency() const { return m_currency; }

  void getBlockchain(std::vector<BlockInfo>& blockchain, const crypto::Hash& head, size_t n) const;
  void getLastNBlockSizes(std::vector<size_t>& blockSizes, const crypto::Hash& head, size_t n) const;
  uint64_t getAlreadyGeneratedCoins(const crypto::Hash& blockId) const;
  uint64_t getAlreadyGeneratedCoins(const cryptonote::Block& blk) const;

  void addBlock(const cryptonote::Block& blk, size_t tsxSize, uint64_t fee, std::vector<size_t>& blockSizes,
    uint64_t alreadyGeneratedCoins);
  bool constructBlock(cryptonote::Block& blk, uint32_t height, const crypto::Hash& previousBlockHash,
    const cryptonote::AccountBase& minerAcc, uint64_t timestamp, uint64_t alreadyGeneratedCoins,
    std::vector<size_t>& blockSizes, const std::list<cryptonote::Transaction>& txList);
  bool constructBlock(cryptonote::Block& blk, const cryptonote::AccountBase& minerAcc, uint64_t timestamp);
  bool constructBlock(cryptonote::Block& blk, const cryptonote::Block& blkPrev, const cryptonote::AccountBase& minerAcc,
    const std::list<cryptonote::Transaction>& txList = std::list<cryptonote::Transaction>());

  bool constructBlockManually(cryptonote::Block& blk, const cryptonote::Block& prevBlock,
    const cryptonote::AccountBase& minerAcc, int actualParams = bf_none, uint8_t majorVer = 0,
    uint8_t minorVer = 0, uint64_t timestamp = 0, const crypto::Hash& previousBlockHash = crypto::Hash(),
    const cryptonote::difficulty_type& diffic = 1, const cryptonote::Transaction& baseTransaction = cryptonote::Transaction(),
    const std::vector<crypto::Hash>& transactionHashes = std::vector<crypto::Hash>(), size_t txsSizes = 0, uint64_t fee = 0);
  bool constructBlockManuallyTx(cryptonote::Block& blk, const cryptonote::Block& prevBlock,
    const cryptonote::AccountBase& minerAcc, const std::vector<crypto::Hash>& transactionHashes, size_t txsSize);
  bool constructMaxSizeBlock(cryptonote::Block& blk, const cryptonote::Block& blkPrev,
    const cryptonote::AccountBase& minerAccount, size_t medianBlockCount = 0,
    const std::list<cryptonote::Transaction>& txList = std::list<cryptonote::Transaction>());

private:
  const cryptonote::Currency& m_currency;
  std::unordered_map<crypto::Hash, BlockInfo> m_blocksInfo;
};

inline cryptonote::difficulty_type getTestDifficulty() { return 1; }
void fillNonce(cryptonote::Block& blk, const cryptonote::difficulty_type& diffic);

bool constructMinerTxManually(const cryptonote::Currency& currency, uint32_t height, uint64_t alreadyGeneratedCoins,
  const cryptonote::AccountPublicAddress& minerAddress, cryptonote::Transaction& tx, uint64_t fee,
  cryptonote::KeyPair* pTxKey = 0);
bool constructMinerTxBySize(const cryptonote::Currency& currency, cryptonote::Transaction& baseTransaction, uint32_t height,
  uint64_t alreadyGeneratedCoins, const cryptonote::AccountPublicAddress& minerAddress,
  std::vector<size_t>& blockSizes, size_t targetTxSize, size_t targetBlockSize, uint64_t fee = 0);
