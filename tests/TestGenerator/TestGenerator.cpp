// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TestGenerator.h"

#include <common/math.hpp>
#include "cryptonote/core/account.h"
#include "cryptonote/core/Miner.h"
#include "cryptonote/core/transaction/TransactionExtra.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/structures/block_entry.h"
#include "cryptonote/structures/array.hpp"

using namespace std;
using namespace cryptonote;

#ifndef CHECK_AND_ASSERT_MES
#define CHECK_AND_ASSERT_MES(expr, fail_ret_val, message)   do{if(!(expr)) {std::cerr << message << std::endl; return fail_ret_val;};}while(0)
#endif


void test_generator::getBlockchain(std::vector<block_info_t>& blockchain, const hash_t& head, size_t n) const {
  hash_t curr = head;
  while (curr != NULL_HASH && blockchain.size() < n) {
    auto it = m_blocksInfo.find(curr);
    if (m_blocksInfo.end() == it) {
      throw std::runtime_error("block hash wasn't found");
    }

    blockchain.push_back(it->second);
    curr = it->second.previousBlockHash;
  }

  std::reverse(blockchain.begin(), blockchain.end());
}

void test_generator::getLastNBlockSizes(std::vector<size_t>& blockSizes, const hash_t& head, size_t n) const {
  std::vector<block_info_t> blockchain;
  getBlockchain(blockchain, head, n);
  for (auto& bi : blockchain) {
    blockSizes.push_back(bi.blockSize);
  }
}

uint64_t test_generator::getAlreadyGeneratedCoins(const hash_t& blockId) const {
  auto it = m_blocksInfo.find(blockId);
  if (it == m_blocksInfo.end()) {
    throw std::runtime_error("block hash wasn't found");
  }

  return it->second.alreadyGeneratedCoins;
}

uint64_t test_generator::getAlreadyGeneratedCoins(const cryptonote::block_t& blk) const {
  hash_t blkHash;
  Block::getHash(blk, blkHash);
  return getAlreadyGeneratedCoins(blkHash);
}

void test_generator::addBlock(const cryptonote::block_t& blk, size_t tsxSize, uint64_t fee,
                              std::vector<size_t>& blockSizes, uint64_t alreadyGeneratedCoins) {
  const size_t blockSize = tsxSize + BinaryArray::size(blk.baseTransaction);
  int64_t emissionChange;
  uint64_t blockReward;
  m_currency.getBlockReward(math::medianValue(blockSizes), blockSize, alreadyGeneratedCoins, fee,
    blockReward, emissionChange);
  m_blocksInfo[Block::getHash(blk)] = block_info_t(blk.previousBlockHash, alreadyGeneratedCoins + emissionChange, blockSize);
}

bool test_generator::constructBlock(cryptonote::block_t& blk, uint32_t height, const hash_t& previousBlockHash,
                                    const cryptonote::Account& minerAcc, uint64_t timestamp, uint64_t alreadyGeneratedCoins,
                                    std::vector<size_t>& blockSizes, const std::list<cryptonote::transaction_t>& txList) {
  blk.majorVersion = defaultMajorVersion;
  blk.minorVersion = defaultMinorVersion;
  blk.timestamp = timestamp;
  blk.previousBlockHash = previousBlockHash;

  blk.transactionHashes.reserve(txList.size());
  for (const transaction_t &tx : txList) {
    hash_t tx_hash;
    BinaryArray::objectHash(tx, tx_hash);
    blk.transactionHashes.push_back(tx_hash);
  }

  uint64_t totalFee = 0;
  size_t txsSize = 0;
  for (auto& tx : txList) {
    uint64_t fee = 0;
    
    bool r = get_tx_fee(tx, fee);
    CHECK_AND_ASSERT_MES(r, false, "wrong transaction passed to construct_block");
    totalFee += fee;
    txsSize += BinaryArray::size(tx);
  }

  blk.baseTransaction = boost::value_initialized<transaction_t>();
  size_t targetBlockSize = txsSize + BinaryArray::size(blk.baseTransaction);
  while (true) {
    if (!m_currency.constructMinerTx(height, math::medianValue(blockSizes), alreadyGeneratedCoins, targetBlockSize,
      totalFee, minerAcc.getAccountKeys().address, blk.baseTransaction, binary_array_t(), 10)) {
      return false;
    }

    size_t actualBlockSize = txsSize + BinaryArray::size(blk.baseTransaction);
    if (targetBlockSize < actualBlockSize) {
      targetBlockSize = actualBlockSize;
    } else if (actualBlockSize < targetBlockSize) {
      size_t delta = targetBlockSize - actualBlockSize;
      blk.baseTransaction.extra.resize(blk.baseTransaction.extra.size() + delta, 0);
      actualBlockSize = txsSize + BinaryArray::size(blk.baseTransaction);
      if (actualBlockSize == targetBlockSize) {
        break;
      } else {
        CHECK_AND_ASSERT_MES(targetBlockSize < actualBlockSize, false, "Unexpected block size");
        delta = actualBlockSize - targetBlockSize;
        blk.baseTransaction.extra.resize(blk.baseTransaction.extra.size() - delta);
        actualBlockSize = txsSize + BinaryArray::size(blk.baseTransaction);
        if (actualBlockSize == targetBlockSize) {
          break;
        } else {
          CHECK_AND_ASSERT_MES(actualBlockSize < targetBlockSize, false, "Unexpected block size");
          blk.baseTransaction.extra.resize(blk.baseTransaction.extra.size() + delta, 0);
          targetBlockSize = txsSize + BinaryArray::size(blk.baseTransaction);
        }
      }
    } else {
      break;
    }
  }

  // Nonce search...
  blk.nonce = 0;
  while (!miner::find_nonce_for_given_block(blk, getTestDifficulty())) {
    blk.timestamp++;
  }

  addBlock(blk, txsSize, totalFee, blockSizes, alreadyGeneratedCoins);

  return true;
}

bool test_generator::constructBlock(cryptonote::block_t& blk, const cryptonote::Account& minerAcc, uint64_t timestamp) {
  std::vector<size_t> blockSizes;
  std::list<cryptonote::transaction_t> txList;
  return constructBlock(blk, 0, NULL_HASH, minerAcc, timestamp, 0, blockSizes, txList);
}

bool test_generator::constructBlock(cryptonote::block_t& blk, const cryptonote::block_t& blkPrev,
                                    const cryptonote::Account& minerAcc,
                                    const std::list<cryptonote::transaction_t>& txList/* = std::list<cryptonote::transaction_t>()*/) {
  uint32_t height = boost::get<base_input_t>(blkPrev.baseTransaction.inputs.front()).blockIndex + 1;
  hash_t previousBlockHash = Block::getHash(blkPrev);
  // Keep difficulty unchanged
  uint64_t timestamp = blkPrev.timestamp + m_currency.difficultyTarget();
  uint64_t alreadyGeneratedCoins = getAlreadyGeneratedCoins(previousBlockHash);
  std::vector<size_t> blockSizes;
  getLastNBlockSizes(blockSizes, previousBlockHash, m_currency.rewardBlocksWindow());

  return constructBlock(blk, height, previousBlockHash, minerAcc, timestamp, alreadyGeneratedCoins, blockSizes, txList);
}

bool test_generator::constructBlockManually(block_t& blk, const block_t& prevBlock, const Account& minerAcc,
                                            int actualParams/* = bf_none*/, uint8_t majorVer/* = 0*/,
                                            uint8_t minorVer/* = 0*/, uint64_t timestamp/* = 0*/,
                                            const hash_t& previousBlockHash/* = hash_t()*/, const difficulty_t& diffic/* = 1*/,
                                            const transaction_t& baseTransaction/* = transaction()*/,
                                            const std::vector<hash_t>& transactionHashes/* = std::vector<hash_t>()*/,
                                            size_t txsSizes/* = 0*/, uint64_t fee/* = 0*/) {
  blk.majorVersion = actualParams & bf_major_ver ? majorVer  : defaultMajorVersion;
  blk.minorVersion = actualParams & bf_minor_ver ? minorVer  : defaultMinorVersion;
  blk.timestamp    = actualParams & bf_timestamp ? timestamp : prevBlock.timestamp + m_currency.difficultyTarget(); // Keep difficulty unchanged
  blk.previousBlockHash       = actualParams & bf_prev_id   ? previousBlockHash    : Block::getHash(prevBlock);
  blk.transactionHashes     = actualParams & bf_tx_hashes ? transactionHashes  : std::vector<hash_t>();
  
  uint32_t height = get_block_height(prevBlock) + 1;
  uint64_t alreadyGeneratedCoins = getAlreadyGeneratedCoins(prevBlock);
  std::vector<size_t> blockSizes;
  getLastNBlockSizes(blockSizes, Block::getHash(prevBlock), m_currency.rewardBlocksWindow());
  if (actualParams & bf_miner_tx) {
    blk.baseTransaction = baseTransaction;
  } else {
    blk.baseTransaction = boost::value_initialized<transaction_t>();
    size_t currentBlockSize = txsSizes + BinaryArray::size(blk.baseTransaction);
    // TODO: This will work, until size of constructed block is less then m_currency.blockGrantedFullRewardZone()
    if (!m_currency.constructMinerTx(height, math::medianValue(blockSizes), alreadyGeneratedCoins, currentBlockSize, 0,
      minerAcc.getAccountKeys().address, blk.baseTransaction, binary_array_t(), 1)) {
        return false;
    }
  }

  difficulty_t aDiffic = actualParams & bf_diffic ? diffic : getTestDifficulty();
  if (1 < aDiffic) {
    fillNonce(blk, aDiffic);
  }

  addBlock(blk, txsSizes, fee, blockSizes, alreadyGeneratedCoins);

  return true;
}

bool test_generator::constructBlockManuallyTx(cryptonote::block_t& blk, const cryptonote::block_t& prevBlock,
                                              const cryptonote::Account& minerAcc,
                                              const std::vector<hash_t>& transactionHashes, size_t txsSize) {
  return constructBlockManually(blk, prevBlock, minerAcc, bf_tx_hashes, 0, 0, 0, hash_t(), 0, transaction_t(),
    transactionHashes, txsSize);
}

bool test_generator::constructMaxSizeBlock(cryptonote::block_t& blk, const cryptonote::block_t& blkPrev,
                                           const cryptonote::Account& minerAccount,
                                           size_t medianBlockCount/* = 0*/,
                                           const std::list<cryptonote::transaction_t>& txList/* = std::list<cryptonote::transaction_t>()*/) {
  std::vector<size_t> blockSizes;
  medianBlockCount = medianBlockCount == 0 ? m_currency.rewardBlocksWindow() : medianBlockCount;
  getLastNBlockSizes(blockSizes, Block::getHash(blkPrev), medianBlockCount);

  size_t median = std::max(math::medianValue(blockSizes), m_currency.blockGrantedFullRewardZone());
  uint64_t totalFee = 0;
  size_t txsSize = 0;
  std::vector<hash_t> transactionHashes;
  for (auto& tx : txList) {
    uint64_t fee = 0;
    bool r = get_tx_fee(tx, fee);
    CHECK_AND_ASSERT_MES(r, false, "wrong transaction passed to construct_max_size_block");
    totalFee += fee;
    txsSize += BinaryArray::size(tx);
    transactionHashes.push_back(BinaryArray::objectHash(tx));
  }

  transaction_t baseTransaction;
  bool r = constructMinerTxBySize(m_currency, baseTransaction, get_block_height(blkPrev) + 1,
    getAlreadyGeneratedCoins(blkPrev), minerAccount.getAccountKeys().address, blockSizes,
    2 * median - txsSize, 2 * median, totalFee);
  if (!r) {
    return false;
  }

  return constructBlockManually(blk, blkPrev, minerAccount, test_generator::bf_miner_tx | test_generator::bf_tx_hashes,
    0, 0, 0, hash_t(), 0, baseTransaction, transactionHashes, txsSize, totalFee);
}

void fillNonce(cryptonote::block_t& blk, const difficulty_t& diffic) {
  blk.nonce = 0;
  while (!miner::find_nonce_for_given_block(blk, diffic)) {
    blk.timestamp++;
  }
}

bool constructMinerTxManually(const cryptonote::Currency& currency, uint32_t height, uint64_t alreadyGeneratedCoins,
                              const account_public_address_t& minerAddress, transaction_t& tx, uint64_t fee,
                              key_pair_t* pTxKey/* = 0*/) {
  // key_pair_t txkey = generateKeyPair();
  key_pair_t txkey = Key::generate();
  addTransactionPublicKeyToExtra(tx.extra, txkey.publicKey);

  if (0 != pTxKey) {
    *pTxKey = txkey;
  }

  base_input_t in;
  in.blockIndex = height;
  tx.inputs.push_back(in);

  // This will work, until size of constructed block is less then currency.blockGrantedFullRewardZone()
  int64_t emissionChange;
  uint64_t blockReward;
  if (!currency.getBlockReward(0, 0, alreadyGeneratedCoins, fee, blockReward, emissionChange)) {
    std::cerr << "Block is too big" << std::endl;
    return false;
  }

  key_derivation_t derivation;
  public_key_t outEphPublicKey;
  generate_key_derivation((const uint8_t*)&minerAddress.viewPublicKey, (const uint8_t*)&txkey.secretKey, (uint8_t*)&derivation);
  derive_public_key((const uint8_t*)&derivation, 0, (const uint8_t*)&minerAddress.spendPublicKey, (uint8_t*)&outEphPublicKey);

  transaction_output_t out;
  out.amount = blockReward;
  out.target = key_output_t{outEphPublicKey};
  tx.outputs.push_back(out);

  tx.version = config::get().transaction.version.major;
  tx.unlockTime = height + currency.minedMoneyUnlockWindow();

  return true;
}

bool constructMinerTxBySize(const cryptonote::Currency& currency, cryptonote::transaction_t& baseTransaction, uint32_t height,
                            uint64_t alreadyGeneratedCoins, const cryptonote::account_public_address_t& minerAddress,
                            std::vector<size_t>& blockSizes, size_t targetTxSize, size_t targetBlockSize,
                            uint64_t fee/* = 0*/) {
  if (!currency.constructMinerTx(height, math::medianValue(blockSizes), alreadyGeneratedCoins, targetBlockSize,
      fee, minerAddress, baseTransaction, binary_array_t(), 1)) {
    return false;
  }

  size_t currentSize = BinaryArray::size(baseTransaction);
  size_t tryCount = 0;
  while (targetTxSize != currentSize) {
    ++tryCount;
    if (10 < tryCount) {
      return false;
    }

    if (targetTxSize < currentSize) {
      size_t diff = currentSize - targetTxSize;
      if (diff <= baseTransaction.extra.size()) {
        baseTransaction.extra.resize(baseTransaction.extra.size() - diff);
      } else {
        return false;
      }
    } else {
      size_t diff = targetTxSize - currentSize;
      baseTransaction.extra.resize(baseTransaction.extra.size() + diff);
    }

    currentSize = BinaryArray::size(baseTransaction);
  }

  return true;
}
