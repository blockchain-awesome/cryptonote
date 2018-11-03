// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TestGenerator.h"

#include <common/Math.h>
#include "cryptonote/core/Account.h"
#include "cryptonote/core/Miner.h"
#include "cryptonote/core/TransactionExtra.h"
#include "cryptonote/core/CryptoNoteTools.h"

using namespace std;
using namespace cryptonote;

#ifndef CHECK_AND_ASSERT_MES
#define CHECK_AND_ASSERT_MES(expr, fail_ret_val, message)   do{if(!(expr)) {std::cerr << message << std::endl; return fail_ret_val;};}while(0)
#endif


void test_generator::getBlockchain(std::vector<BlockInfo>& blockchain, const crypto::hash_t& head, size_t n) const {
  crypto::hash_t curr = head;
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

void test_generator::getLastNBlockSizes(std::vector<size_t>& blockSizes, const crypto::hash_t& head, size_t n) const {
  std::vector<BlockInfo> blockchain;
  getBlockchain(blockchain, head, n);
  for (auto& bi : blockchain) {
    blockSizes.push_back(bi.blockSize);
  }
}

uint64_t test_generator::getAlreadyGeneratedCoins(const crypto::hash_t& blockId) const {
  auto it = m_blocksInfo.find(blockId);
  if (it == m_blocksInfo.end()) {
    throw std::runtime_error("block hash wasn't found");
  }

  return it->second.alreadyGeneratedCoins;
}

uint64_t test_generator::getAlreadyGeneratedCoins(const cryptonote::block_t& blk) const {
  crypto::hash_t blkHash;
  get_block_hash(blk, blkHash);
  return getAlreadyGeneratedCoins(blkHash);
}

void test_generator::addBlock(const cryptonote::block_t& blk, size_t tsxSize, uint64_t fee,
                              std::vector<size_t>& blockSizes, uint64_t alreadyGeneratedCoins) {
  const size_t blockSize = tsxSize + getObjectBinarySize(blk.baseTransaction);
  int64_t emissionChange;
  uint64_t blockReward;
  m_currency.getBlockReward(Common::medianValue(blockSizes), blockSize, alreadyGeneratedCoins, fee,
    blockReward, emissionChange);
  m_blocksInfo[get_block_hash(blk)] = BlockInfo(blk.previousBlockHash, alreadyGeneratedCoins + emissionChange, blockSize);
}

bool test_generator::constructBlock(cryptonote::block_t& blk, uint32_t height, const crypto::hash_t& previousBlockHash,
                                    const cryptonote::AccountBase& minerAcc, uint64_t timestamp, uint64_t alreadyGeneratedCoins,
                                    std::vector<size_t>& blockSizes, const std::list<cryptonote::Transaction>& txList) {
  blk.majorVersion = defaultMajorVersion;
  blk.minorVersion = defaultMinorVersion;
  blk.timestamp = timestamp;
  blk.previousBlockHash = previousBlockHash;

  blk.transactionHashes.reserve(txList.size());
  for (const Transaction &tx : txList) {
    crypto::hash_t tx_hash;
    getObjectHash(tx, tx_hash);
    blk.transactionHashes.push_back(tx_hash);
  }

  uint64_t totalFee = 0;
  size_t txsSize = 0;
  for (auto& tx : txList) {
    uint64_t fee = 0;
    
    bool r = get_tx_fee(tx, fee);
    CHECK_AND_ASSERT_MES(r, false, "wrong transaction passed to construct_block");
    totalFee += fee;
    txsSize += getObjectBinarySize(tx);
  }

  blk.baseTransaction = boost::value_initialized<Transaction>();
  size_t targetBlockSize = txsSize + getObjectBinarySize(blk.baseTransaction);
  while (true) {
    if (!m_currency.constructMinerTx(height, Common::medianValue(blockSizes), alreadyGeneratedCoins, targetBlockSize,
      totalFee, minerAcc.getAccountKeys().address, blk.baseTransaction, BinaryArray(), 10)) {
      return false;
    }

    size_t actualBlockSize = txsSize + getObjectBinarySize(blk.baseTransaction);
    if (targetBlockSize < actualBlockSize) {
      targetBlockSize = actualBlockSize;
    } else if (actualBlockSize < targetBlockSize) {
      size_t delta = targetBlockSize - actualBlockSize;
      blk.baseTransaction.extra.resize(blk.baseTransaction.extra.size() + delta, 0);
      actualBlockSize = txsSize + getObjectBinarySize(blk.baseTransaction);
      if (actualBlockSize == targetBlockSize) {
        break;
      } else {
        CHECK_AND_ASSERT_MES(targetBlockSize < actualBlockSize, false, "Unexpected block size");
        delta = actualBlockSize - targetBlockSize;
        blk.baseTransaction.extra.resize(blk.baseTransaction.extra.size() - delta);
        actualBlockSize = txsSize + getObjectBinarySize(blk.baseTransaction);
        if (actualBlockSize == targetBlockSize) {
          break;
        } else {
          CHECK_AND_ASSERT_MES(actualBlockSize < targetBlockSize, false, "Unexpected block size");
          blk.baseTransaction.extra.resize(blk.baseTransaction.extra.size() + delta, 0);
          targetBlockSize = txsSize + getObjectBinarySize(blk.baseTransaction);
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

bool test_generator::constructBlock(cryptonote::block_t& blk, const cryptonote::AccountBase& minerAcc, uint64_t timestamp) {
  std::vector<size_t> blockSizes;
  std::list<cryptonote::Transaction> txList;
  return constructBlock(blk, 0, NULL_HASH, minerAcc, timestamp, 0, blockSizes, txList);
}

bool test_generator::constructBlock(cryptonote::block_t& blk, const cryptonote::block_t& blkPrev,
                                    const cryptonote::AccountBase& minerAcc,
                                    const std::list<cryptonote::Transaction>& txList/* = std::list<cryptonote::Transaction>()*/) {
  uint32_t height = boost::get<BaseInput>(blkPrev.baseTransaction.inputs.front()).blockIndex + 1;
  crypto::hash_t previousBlockHash = get_block_hash(blkPrev);
  // Keep difficulty unchanged
  uint64_t timestamp = blkPrev.timestamp + m_currency.difficultyTarget();
  uint64_t alreadyGeneratedCoins = getAlreadyGeneratedCoins(previousBlockHash);
  std::vector<size_t> blockSizes;
  getLastNBlockSizes(blockSizes, previousBlockHash, m_currency.rewardBlocksWindow());

  return constructBlock(blk, height, previousBlockHash, minerAcc, timestamp, alreadyGeneratedCoins, blockSizes, txList);
}

bool test_generator::constructBlockManually(block_t& blk, const block_t& prevBlock, const AccountBase& minerAcc,
                                            int actualParams/* = bf_none*/, uint8_t majorVer/* = 0*/,
                                            uint8_t minorVer/* = 0*/, uint64_t timestamp/* = 0*/,
                                            const crypto::hash_t& previousBlockHash/* = crypto::hash_t()*/, const difficulty_type& diffic/* = 1*/,
                                            const Transaction& baseTransaction/* = transaction()*/,
                                            const std::vector<crypto::hash_t>& transactionHashes/* = std::vector<crypto::hash_t>()*/,
                                            size_t txsSizes/* = 0*/, uint64_t fee/* = 0*/) {
  blk.majorVersion = actualParams & bf_major_ver ? majorVer  : defaultMajorVersion;
  blk.minorVersion = actualParams & bf_minor_ver ? minorVer  : defaultMinorVersion;
  blk.timestamp    = actualParams & bf_timestamp ? timestamp : prevBlock.timestamp + m_currency.difficultyTarget(); // Keep difficulty unchanged
  blk.previousBlockHash       = actualParams & bf_prev_id   ? previousBlockHash    : get_block_hash(prevBlock);
  blk.transactionHashes     = actualParams & bf_tx_hashes ? transactionHashes  : std::vector<crypto::hash_t>();
  
  uint32_t height = get_block_height(prevBlock) + 1;
  uint64_t alreadyGeneratedCoins = getAlreadyGeneratedCoins(prevBlock);
  std::vector<size_t> blockSizes;
  getLastNBlockSizes(blockSizes, get_block_hash(prevBlock), m_currency.rewardBlocksWindow());
  if (actualParams & bf_miner_tx) {
    blk.baseTransaction = baseTransaction;
  } else {
    blk.baseTransaction = boost::value_initialized<Transaction>();
    size_t currentBlockSize = txsSizes + getObjectBinarySize(blk.baseTransaction);
    // TODO: This will work, until size of constructed block is less then m_currency.blockGrantedFullRewardZone()
    if (!m_currency.constructMinerTx(height, Common::medianValue(blockSizes), alreadyGeneratedCoins, currentBlockSize, 0,
      minerAcc.getAccountKeys().address, blk.baseTransaction, BinaryArray(), 1)) {
        return false;
    }
  }

  difficulty_type aDiffic = actualParams & bf_diffic ? diffic : getTestDifficulty();
  if (1 < aDiffic) {
    fillNonce(blk, aDiffic);
  }

  addBlock(blk, txsSizes, fee, blockSizes, alreadyGeneratedCoins);

  return true;
}

bool test_generator::constructBlockManuallyTx(cryptonote::block_t& blk, const cryptonote::block_t& prevBlock,
                                              const cryptonote::AccountBase& minerAcc,
                                              const std::vector<crypto::hash_t>& transactionHashes, size_t txsSize) {
  return constructBlockManually(blk, prevBlock, minerAcc, bf_tx_hashes, 0, 0, 0, crypto::hash_t(), 0, Transaction(),
    transactionHashes, txsSize);
}

bool test_generator::constructMaxSizeBlock(cryptonote::block_t& blk, const cryptonote::block_t& blkPrev,
                                           const cryptonote::AccountBase& minerAccount,
                                           size_t medianBlockCount/* = 0*/,
                                           const std::list<cryptonote::Transaction>& txList/* = std::list<cryptonote::Transaction>()*/) {
  std::vector<size_t> blockSizes;
  medianBlockCount = medianBlockCount == 0 ? m_currency.rewardBlocksWindow() : medianBlockCount;
  getLastNBlockSizes(blockSizes, get_block_hash(blkPrev), medianBlockCount);

  size_t median = std::max(Common::medianValue(blockSizes), m_currency.blockGrantedFullRewardZone());
  uint64_t totalFee = 0;
  size_t txsSize = 0;
  std::vector<crypto::hash_t> transactionHashes;
  for (auto& tx : txList) {
    uint64_t fee = 0;
    bool r = get_tx_fee(tx, fee);
    CHECK_AND_ASSERT_MES(r, false, "wrong transaction passed to construct_max_size_block");
    totalFee += fee;
    txsSize += getObjectBinarySize(tx);
    transactionHashes.push_back(getObjectHash(tx));
  }

  Transaction baseTransaction;
  bool r = constructMinerTxBySize(m_currency, baseTransaction, get_block_height(blkPrev) + 1,
    getAlreadyGeneratedCoins(blkPrev), minerAccount.getAccountKeys().address, blockSizes,
    2 * median - txsSize, 2 * median, totalFee);
  if (!r) {
    return false;
  }

  return constructBlockManually(blk, blkPrev, minerAccount, test_generator::bf_miner_tx | test_generator::bf_tx_hashes,
    0, 0, 0, crypto::hash_t(), 0, baseTransaction, transactionHashes, txsSize, totalFee);
}

void fillNonce(cryptonote::block_t& blk, const difficulty_type& diffic) {
  blk.nonce = 0;
  while (!miner::find_nonce_for_given_block(blk, diffic)) {
    blk.timestamp++;
  }
}

bool constructMinerTxManually(const cryptonote::Currency& currency, uint32_t height, uint64_t alreadyGeneratedCoins,
                              const AccountPublicAddress& minerAddress, Transaction& tx, uint64_t fee,
                              KeyPair* pTxKey/* = 0*/) {
  // KeyPair txkey = generateKeyPair();
  KeyPair txkey = Key::generate();
  addTransactionPublicKeyToExtra(tx.extra, txkey.publicKey);

  if (0 != pTxKey) {
    *pTxKey = txkey;
  }

  BaseInput in;
  in.blockIndex = height;
  tx.inputs.push_back(in);

  // This will work, until size of constructed block is less then currency.blockGrantedFullRewardZone()
  int64_t emissionChange;
  uint64_t blockReward;
  if (!currency.getBlockReward(0, 0, alreadyGeneratedCoins, fee, blockReward, emissionChange)) {
    std::cerr << "Block is too big" << std::endl;
    return false;
  }

  crypto::key_derivation_t derivation;
  crypto::public_key_t outEphPublicKey;
  crypto::generate_key_derivation(minerAddress.viewPublicKey, txkey.secretKey, derivation);
  crypto::derive_public_key(derivation, 0, minerAddress.spendPublicKey, outEphPublicKey);

  TransactionOutput out;
  out.amount = blockReward;
  out.target = KeyOutput{outEphPublicKey};
  tx.outputs.push_back(out);

  tx.version = CURRENT_TRANSACTION_VERSION;
  tx.unlockTime = height + currency.minedMoneyUnlockWindow();

  return true;
}

bool constructMinerTxBySize(const cryptonote::Currency& currency, cryptonote::Transaction& baseTransaction, uint32_t height,
                            uint64_t alreadyGeneratedCoins, const cryptonote::AccountPublicAddress& minerAddress,
                            std::vector<size_t>& blockSizes, size_t targetTxSize, size_t targetBlockSize,
                            uint64_t fee/* = 0*/) {
  if (!currency.constructMinerTx(height, Common::medianValue(blockSizes), alreadyGeneratedCoins, targetBlockSize,
      fee, minerAddress, baseTransaction, cryptonote::BinaryArray(), 1)) {
    return false;
  }

  size_t currentSize = getObjectBinarySize(baseTransaction);
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

    currentSize = getObjectBinarySize(baseTransaction);
  }

  return true;
}
