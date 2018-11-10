// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"

#include <system_error>

#include <boost/range/combine.hpp>

#include "EventWaiter.h"
#include "ICoreStub.h"
#include "ICryptoNoteProtocolQueryStub.h"
#include "InProcessNode/InProcessNode.h"
#include "TestBlockchainGenerator.h"
#include "logging/FileLogger.h"
#include "cryptonote/core/TransactionApi.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/VerificationContext.h"
#include "common/StringTools.h"

using namespace crypto;
using namespace cryptonote;
using namespace Common;

struct CallbackStatus {
  CallbackStatus() {}

  bool wait() { return waiter.wait_for(std::chrono::milliseconds(3000)); }
  bool ok() { return waiter.wait_for(std::chrono::milliseconds(3000)) && !static_cast<bool>(code); }
  void setStatus(const std::error_code& ec) { code = ec; waiter.notify(); }
  std::error_code getStatus() const { return code; }

  std::error_code code;
  EventWaiter waiter;
};

namespace {
cryptonote::transaction_t createTx(cryptonote::ITransactionReader& tx) {
  cryptonote::transaction_t outTx;
  fromBinaryArray(outTx, tx.getTransactionData());
  return outTx;
}
}

class InProcessNodeTests : public ::testing::Test {
public:
  InProcessNodeTests() :
    node(coreStub, protocolQueryStub),
    currency(cryptonote::CurrencyBuilder(os::appdata::path(), config::testnet::data, logger).currency()),
    generator(currency) {}
  void SetUp() override;

protected:
  void initNode();

  ICoreStub coreStub;
  ICryptoNoteProtocolQueryStub protocolQueryStub;
  cryptonote::InProcessNode node;

  cryptonote::Currency currency;
  TestBlockchainGenerator generator;
  Logging::FileLogger logger;
};

void InProcessNodeTests::SetUp() {
  logger.init("/dev/null");
  initNode();
}

void InProcessNodeTests::initNode() {
  CallbackStatus status;

  node.init([&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.ok());
}

TEST_F(InProcessNodeTests, initOk) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);
  CallbackStatus status;

  newNode.init([&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.ok());
}

TEST_F(InProcessNodeTests, doubleInit) {
  CallbackStatus status;
  node.init([&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());

  std::error_code ec = status.getStatus();
  ASSERT_NE(ec, std::error_code());
}

TEST_F(InProcessNodeTests, shutdownNotInited) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);
  ASSERT_FALSE(newNode.shutdown());
}

TEST_F(InProcessNodeTests, shutdown) {
  ASSERT_TRUE(node.shutdown());
}

TEST_F(InProcessNodeTests, getPeersCountSuccess) {
  protocolQueryStub.setPeerCount(1);
  ASSERT_EQ(1, node.getPeerCount());
}

TEST_F(InProcessNodeTests, getLastLocalBlockHeightSuccess) {
  crypto::hash_t ignore;
  coreStub.set_blockchain_top(10, ignore);

  ASSERT_EQ(10, node.getLastLocalBlockHeight());
}

TEST_F(InProcessNodeTests, getLastKnownBlockHeightSuccess) {
  protocolQueryStub.setObservedHeight(10);
  ASSERT_EQ(10, node.getLastKnownBlockHeight() + 1);
}

TEST_F(InProcessNodeTests, getTransactionOutsGlobalIndicesSuccess) {
  crypto::hash_t ignore;
  std::vector<uint32_t> indices;
  std::vector<uint32_t> expectedIndices;

  uint64_t start = 10;
  std::generate_n(std::back_inserter(expectedIndices), 5, [&start] () { return start++; });
  coreStub.set_outputs_gindexs(expectedIndices, true);

  CallbackStatus status;
  node.getTransactionOutsGlobalIndices(ignore, indices, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.ok());

  ASSERT_EQ(expectedIndices.size(), indices.size());
  std::sort(indices.begin(), indices.end());
  ASSERT_TRUE(std::equal(indices.begin(), indices.end(), expectedIndices.begin()));
}

TEST_F(InProcessNodeTests, getTransactionOutsGlobalIndicesFailure) {
  crypto::hash_t ignore;
  std::vector<uint32_t> indices;
  coreStub.set_outputs_gindexs(indices, false);

  CallbackStatus status;
  node.getTransactionOutsGlobalIndices(ignore, indices, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getRandomOutsByAmountsSuccess) {
  crypto::public_key_t ignoredPublicKey;
  crypto::secret_key_t ignoredSectetKey;
  crypto::generate_keys(ignoredPublicKey, ignoredSectetKey);

  cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_response expectedResp;
  cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_outs_for_amount out;
  out.amount = 10;
  out.outs.push_back({ 11, ignoredPublicKey });
  expectedResp.outs.push_back(out);
  coreStub.set_random_outs(expectedResp, true);

  std::vector<cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_outs_for_amount> outs;

  CallbackStatus status;
  node.getRandomOutsByAmounts({1,2,3}, 1, outs, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.ok());
  ASSERT_EQ(1, outs.size());

  ASSERT_EQ(10, outs[0].amount);
  ASSERT_EQ(1, outs[0].outs.size());
  ASSERT_EQ(11, outs[0].outs.front().global_amount_index);
}

TEST_F(InProcessNodeTests, getRandomOutsByAmountsFailure) {
  cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_response expectedResp;
  coreStub.set_random_outs(expectedResp, false);

  std::vector<cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_outs_for_amount> outs;

  CallbackStatus status;
  node.getRandomOutsByAmounts({1,2,3}, 1, outs, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getPeerCountUninitialized) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);
  ASSERT_ANY_THROW(newNode.getPeerCount());
}

TEST_F(InProcessNodeTests, getLastLocalBlockHeightUninitialized) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);
  ASSERT_ANY_THROW(newNode.getLastLocalBlockHeight());
}

TEST_F(InProcessNodeTests, getLastKnownBlockHeightUninitialized) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);
  ASSERT_ANY_THROW(newNode.getLastKnownBlockHeight());
}

TEST_F(InProcessNodeTests, getNewBlocksUninitialized) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);
  std::vector<crypto::hash_t> knownBlockIds;
  std::vector<cryptonote::block_complete_entry> newBlocks;
  uint32_t startHeight;

  CallbackStatus status;
  newNode.getNewBlocks(std::move(knownBlockIds), newBlocks, startHeight, [&] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getTransactionOutsGlobalIndicesUninitialized) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);
  std::vector<uint32_t> outsGlobalIndices;

  CallbackStatus status;
  newNode.getTransactionOutsGlobalIndices(crypto::hash_t(), outsGlobalIndices, [&] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getRandomOutsByAmountsUninitialized) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);
  std::vector<cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_outs_for_amount> outs;

  CallbackStatus status;
  newNode.getRandomOutsByAmounts({1,2,3}, 1, outs, [&] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, relayTransactionUninitialized) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);

  CallbackStatus status;
  newNode.relayTransaction(cryptonote::transaction_t(), [&] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getBlocksByHeightEmpty) {
  std::vector<uint32_t> blockHeights;
  std::vector<std::vector<cryptonote::BlockDetails>> blocks;
  ASSERT_EQ(blockHeights.size(), 0);
  ASSERT_EQ(blocks.size(), 0);

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  CallbackStatus status;
  node.getBlocks(blockHeights, blocks, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_EQ(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getBlocksByHeightMany) {
  const size_t NUMBER_OF_BLOCKS = 10;

  std::vector<uint32_t> blockHeights;
  std::vector<std::vector<cryptonote::BlockDetails>> actualBlocks;

  std::vector<cryptonote::block_t> expectedBlocks;

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  generator.generateEmptyBlocks(NUMBER_OF_BLOCKS);
  ASSERT_GE(generator.getBlockchain().size(), NUMBER_OF_BLOCKS);

  for (auto iter = generator.getBlockchain().begin() + 1; iter != generator.getBlockchain().end(); iter++) {
    expectedBlocks.push_back(*iter);
    blockHeights.push_back(std::move(boost::get<cryptonote::base_input_t>(iter->baseTransaction.inputs.front()).blockIndex));
    coreStub.addBlock(*iter);
  }

  ASSERT_GE(blockHeights.size(), NUMBER_OF_BLOCKS);
  ASSERT_EQ(blockHeights.size(), expectedBlocks.size());
  ASSERT_EQ(actualBlocks.size(), 0);

  CallbackStatus status;
  node.getBlocks(blockHeights, actualBlocks, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_EQ(std::error_code(), status.getStatus());

  ASSERT_EQ(blockHeights.size(), expectedBlocks.size());
  ASSERT_EQ(blockHeights.size(), actualBlocks.size());
  auto range1 = boost::combine(blockHeights, expectedBlocks);
  auto range = boost::combine(range1, actualBlocks);
  for (const boost::tuple<boost::tuple<size_t, cryptonote::block_t>, std::vector<cryptonote::BlockDetails>>& sameHeight : range) {
    EXPECT_EQ(sameHeight.get<1>().size(), 1);
    for (const cryptonote::BlockDetails& block : sameHeight.get<1>()) {
      EXPECT_EQ(block.height, sameHeight.get<0>().get<0>());
      crypto::hash_t expectedCryptoHash = cryptonote::get_block_hash(sameHeight.get<0>().get<1>());
      hash_t expectedHash = reinterpret_cast<const hash_t&>(expectedCryptoHash);
      EXPECT_EQ(block.hash, expectedHash);
      EXPECT_FALSE(block.isOrphaned);
    }
  }
}

TEST_F(InProcessNodeTests, getBlocksByHeightFail) {
  const size_t NUMBER_OF_BLOCKS = 10;

  std::vector<uint32_t> blockHeights;
  std::vector<std::vector<cryptonote::BlockDetails>> actualBlocks;

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  generator.generateEmptyBlocks(NUMBER_OF_BLOCKS);
  ASSERT_LT(generator.getBlockchain().size(), NUMBER_OF_BLOCKS * 2);

  for (const cryptonote::block_t& block : generator.getBlockchain()) {
    coreStub.addBlock(block);
  }

  for (uint32_t i = 0; i < NUMBER_OF_BLOCKS * 2; ++i) {
    blockHeights.push_back(std::move(i));
  }

  ASSERT_EQ(actualBlocks.size(), 0);

  CallbackStatus status;
  node.getBlocks(blockHeights, actualBlocks, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getBlocksByHeightNotInited) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);

  std::vector<uint32_t> blockHeights;
  std::vector<std::vector<cryptonote::BlockDetails>> blocks;
  ASSERT_EQ(blockHeights.size(), 0);
  ASSERT_EQ(blocks.size(), 0);

  CallbackStatus status;
  newNode.getBlocks(blockHeights, blocks, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getBlocksByHashEmpty) {
  std::vector<crypto::hash_t> blockHashes;
  std::vector<cryptonote::BlockDetails> blocks;
  ASSERT_EQ(blockHashes.size(), 0);
  ASSERT_EQ(blocks.size(), 0);

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  CallbackStatus status;
  node.getBlocks(blockHashes, blocks, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_EQ(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getBlocksByHashMany) {
  const size_t NUMBER_OF_BLOCKS = 10;

  std::vector<crypto::hash_t> blockHashes;
  std::vector<cryptonote::BlockDetails> actualBlocks;

  std::vector<cryptonote::block_t> expectedBlocks;

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  generator.generateEmptyBlocks(NUMBER_OF_BLOCKS);
  ASSERT_GE(generator.getBlockchain().size(), NUMBER_OF_BLOCKS);

  for (auto iter = generator.getBlockchain().begin() + 1; iter != generator.getBlockchain().end(); iter++) {
    expectedBlocks.push_back(*iter);
    blockHashes.push_back(cryptonote::get_block_hash(*iter));
    coreStub.addBlock(*iter);
  }

  ASSERT_GE(blockHashes.size(), NUMBER_OF_BLOCKS);
  ASSERT_EQ(blockHashes.size(), expectedBlocks.size());
  ASSERT_EQ(actualBlocks.size(), 0);

  CallbackStatus status;
  node.getBlocks(blockHashes, actualBlocks, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_EQ(std::error_code(), status.getStatus());

  ASSERT_EQ(blockHashes.size(), expectedBlocks.size());
  ASSERT_EQ(blockHashes.size(), actualBlocks.size());
  auto range1 = boost::combine(blockHashes, expectedBlocks);
  auto range = boost::combine(range1, actualBlocks);
  for (const boost::tuple<boost::tuple<crypto::hash_t, cryptonote::block_t>, cryptonote::BlockDetails>& sameHeight : range) {
    crypto::hash_t expectedCryptoHash = cryptonote::get_block_hash(sameHeight.get<0>().get<1>());
    EXPECT_EQ(expectedCryptoHash, sameHeight.get<0>().get<0>());
    hash_t expectedHash = reinterpret_cast<const hash_t&>(expectedCryptoHash);
    EXPECT_EQ(sameHeight.get<1>().hash, expectedHash);
    EXPECT_FALSE(sameHeight.get<1>().isOrphaned);
  }
}

TEST_F(InProcessNodeTests, getBlocksByHashFail) {
  const size_t NUMBER_OF_BLOCKS = 10;

  std::vector<crypto::hash_t> blockHashes;
  std::vector<cryptonote::BlockDetails> actualBlocks;

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  generator.generateEmptyBlocks(NUMBER_OF_BLOCKS);
  ASSERT_LT(generator.getBlockchain().size(), NUMBER_OF_BLOCKS * 2);

  for (const cryptonote::block_t& block : generator.getBlockchain()) {
    coreStub.addBlock(block);
  }

  for (uint32_t i = 0; i < NUMBER_OF_BLOCKS * 2; ++i) {
    blockHashes.push_back(boost::value_initialized<crypto::hash_t>());
  }

  ASSERT_EQ(actualBlocks.size(), 0);

  CallbackStatus status;
  node.getBlocks(blockHashes, actualBlocks, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getBlocksByHashNotInited) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);

  std::vector<crypto::hash_t> blockHashes;
  std::vector<cryptonote::BlockDetails> blocks;
  ASSERT_EQ(blockHashes.size(), 0);
  ASSERT_EQ(blocks.size(), 0);

  CallbackStatus status;
  newNode.getBlocks(blockHashes, blocks, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getTxEmpty) {
  std::vector<crypto::hash_t> transactionHashes;
  std::vector<cryptonote::transaction_details_t> transactions;
  ASSERT_EQ(transactionHashes.size(), 0);
  ASSERT_EQ(transactions.size(), 0);

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  CallbackStatus status;
  node.getTransactions(transactionHashes, transactions, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_EQ(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getTxMany) {
  size_t POOL_TX_NUMBER = 10;
  size_t BLOCKCHAIN_TX_NUMBER = 10;

  std::vector<crypto::hash_t> transactionHashes;
  std::vector<cryptonote::transaction_details_t> actualTransactions;

  std::vector<std::tuple<cryptonote::transaction_t, crypto::hash_t, uint64_t>> expectedTransactions;

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  size_t prevBlockchainSize = generator.getBlockchain().size();
  for (size_t i = 0; i < BLOCKCHAIN_TX_NUMBER; ++i) {
    auto txptr = cryptonote::createTransaction();
    auto tx = ::createTx(*txptr.get());
    transactionHashes.push_back(cryptonote::getObjectHash(tx));
    generator.addTxToBlockchain(tx);
    ASSERT_EQ(generator.getBlockchain().size(), prevBlockchainSize + 1);
    prevBlockchainSize = generator.getBlockchain().size();
    coreStub.addBlock(generator.getBlockchain().back());
    coreStub.addTransaction(tx);
    expectedTransactions.push_back(std::make_tuple(tx, cryptonote::get_block_hash(generator.getBlockchain().back()), boost::get<cryptonote::base_input_t>(generator.getBlockchain().back().baseTransaction.inputs.front()).blockIndex));
  }

  ASSERT_EQ(transactionHashes.size(), BLOCKCHAIN_TX_NUMBER);
  ASSERT_EQ(transactionHashes.size(), expectedTransactions.size());
  ASSERT_EQ(actualTransactions.size(), 0);

  for (size_t i = 0; i < POOL_TX_NUMBER; ++i) {
    auto txptr = cryptonote::createTransaction();
    auto tx = ::createTx(*txptr.get());
    transactionHashes.push_back(cryptonote::getObjectHash(tx));
    coreStub.addTransaction(tx);
    expectedTransactions.push_back(std::make_tuple(tx, boost::value_initialized<crypto::hash_t>(), boost::value_initialized<uint64_t>()));
  }

  ASSERT_EQ(transactionHashes.size(), BLOCKCHAIN_TX_NUMBER + POOL_TX_NUMBER);
  ASSERT_EQ(transactionHashes.size(), expectedTransactions.size());
  ASSERT_EQ(actualTransactions.size(), 0);


  CallbackStatus status;
  node.getTransactions(transactionHashes, actualTransactions, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_EQ(std::error_code(), status.getStatus());

  ASSERT_EQ(transactionHashes.size(), expectedTransactions.size());
  ASSERT_EQ(transactionHashes.size(), actualTransactions.size());
  auto range1 = boost::combine(transactionHashes, actualTransactions);
  auto range = boost::combine(range1, expectedTransactions);
  for (const boost::tuple<boost::tuple<crypto::hash_t, cryptonote::transaction_details_t>, std::tuple<cryptonote::transaction_t, crypto::hash_t, uint64_t>>& sameHeight : range) {
    crypto::hash_t expectedCryptoHash = cryptonote::getObjectHash(std::get<0>(sameHeight.get<1>()));
    EXPECT_EQ(expectedCryptoHash, sameHeight.get<0>().get<0>());
    hash_t expectedHash = reinterpret_cast<const hash_t&>(expectedCryptoHash);
    EXPECT_EQ(sameHeight.get<0>().get<1>().hash, expectedHash);
    if (std::get<1>(sameHeight.get<1>()) != boost::value_initialized<crypto::hash_t>()) {
      EXPECT_TRUE(sameHeight.get<0>().get<1>().inBlockchain);
      hash_t expectedBlockHash = reinterpret_cast<const hash_t&>(std::get<1>(sameHeight.get<1>()));
      EXPECT_EQ(sameHeight.get<0>().get<1>().blockHash, expectedBlockHash);
      EXPECT_EQ(sameHeight.get<0>().get<1>().blockHeight, std::get<2>(sameHeight.get<1>()));
    } else {
      EXPECT_FALSE(sameHeight.get<0>().get<1>().inBlockchain);
    }
  }
}

TEST_F(InProcessNodeTests, getTxFail) {
  size_t POOL_TX_NUMBER = 10;
  size_t BLOCKCHAIN_TX_NUMBER = 10;

  std::vector<crypto::hash_t> transactionHashes;
  std::vector<cryptonote::transaction_details_t> actualTransactions;

  std::vector<std::tuple<cryptonote::transaction_t, crypto::hash_t, uint64_t>> expectedTransactions;

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  size_t prevBlockchainSize = generator.getBlockchain().size();
  for (size_t i = 0; i < BLOCKCHAIN_TX_NUMBER; ++i) {
    auto txptr = cryptonote::createTransaction();
    auto tx = ::createTx(*txptr.get());
    transactionHashes.push_back(cryptonote::getObjectHash(tx));
    generator.addTxToBlockchain(tx);
    ASSERT_EQ(generator.getBlockchain().size(), prevBlockchainSize + 1);
    prevBlockchainSize = generator.getBlockchain().size();
    coreStub.addBlock(generator.getBlockchain().back());
    coreStub.addTransaction(tx);
    expectedTransactions.push_back(std::make_tuple(tx, cryptonote::get_block_hash(generator.getBlockchain().back()), boost::get<cryptonote::base_input_t>(generator.getBlockchain().back().baseTransaction.inputs.front()).blockIndex));
  }

  ASSERT_EQ(transactionHashes.size(), BLOCKCHAIN_TX_NUMBER);
  ASSERT_EQ(transactionHashes.size(), expectedTransactions.size());
  ASSERT_EQ(actualTransactions.size(), 0);

  for (size_t i = 0; i < POOL_TX_NUMBER; ++i) {
    auto txptr = cryptonote::createTransaction();
    auto tx = ::createTx(*txptr.get());
    transactionHashes.push_back(cryptonote::getObjectHash(tx));
    expectedTransactions.push_back(std::make_tuple(tx, boost::value_initialized<crypto::hash_t>(), boost::value_initialized<uint64_t>()));
  }

  ASSERT_EQ(transactionHashes.size(), BLOCKCHAIN_TX_NUMBER + POOL_TX_NUMBER);
  ASSERT_EQ(transactionHashes.size(), expectedTransactions.size());
  ASSERT_EQ(actualTransactions.size(), 0);


  CallbackStatus status;
  node.getTransactions(transactionHashes, actualTransactions, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());

}

TEST_F(InProcessNodeTests, getTxNotInited) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);

  std::vector<crypto::hash_t> transactionHashes;
  std::vector<cryptonote::transaction_details_t> transactions;
  ASSERT_EQ(transactionHashes.size(), 0);
  ASSERT_EQ(transactions.size(), 0);

  coreStub.set_blockchain_top(0, boost::value_initialized<crypto::hash_t>());

  CallbackStatus status;
  newNode.getTransactions(transactionHashes, transactions, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, isSynchronized) {
  bool syncStatus;
  {
    CallbackStatus status;
    node.isSynchronized(syncStatus, [&status] (std::error_code ec) { status.setStatus(ec); });
    ASSERT_TRUE(status.wait());
    ASSERT_EQ(std::error_code(), status.getStatus());
    ASSERT_FALSE(syncStatus);
  }

  protocolQueryStub.setSynchronizedStatus(true);

  {
    CallbackStatus status;
    node.isSynchronized(syncStatus, [&status] (std::error_code ec) { status.setStatus(ec); });
    ASSERT_TRUE(status.wait());
    ASSERT_EQ(std::error_code(), status.getStatus());
    ASSERT_TRUE(syncStatus);
  }
}

TEST_F(InProcessNodeTests, isSynchronizedNotInited) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);
  bool syncStatus;

  CallbackStatus status;
  newNode.isSynchronized(syncStatus, [&status] (std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getLastLocalBlockTimestamp) {
  class GetBlockTimestampCore : public ICoreStub {
  public:
    GetBlockTimestampCore(uint64_t timestamp) : timestamp(timestamp) {}
    virtual void get_blockchain_top(uint32_t& height, crypto::hash_t& top_id) override {
    }

    virtual bool getBlockByHash(const crypto::hash_t &h, cryptonote::block_t &blk) override {
      blk.timestamp = timestamp;
      return true;
    }

    uint64_t timestamp;
  };

  uint64_t expectedTimestamp = 1234567890;
  GetBlockTimestampCore core(expectedTimestamp);
  cryptonote::InProcessNode newNode(core, protocolQueryStub);

  CallbackStatus initStatus;
  newNode.init([&initStatus] (std::error_code ec) { initStatus.setStatus(ec); });
  ASSERT_TRUE(initStatus.wait());

  uint64_t timestamp = newNode.getLastLocalBlockTimestamp();

  ASSERT_EQ(expectedTimestamp, timestamp);
}

TEST_F(InProcessNodeTests, getLastLocalBlockTimestampError) {
  class GetBlockTimestampErrorCore : public ICoreStub {
  public:
    virtual void get_blockchain_top(uint32_t& height, crypto::hash_t& top_id) override {
    }

    virtual bool getBlockByHash(const crypto::hash_t &h, cryptonote::block_t &blk) override {
      return false;
    }
  };

  GetBlockTimestampErrorCore core;
  cryptonote::InProcessNode newNode(core, protocolQueryStub);

  CallbackStatus initStatus;
  newNode.init([&initStatus] (std::error_code ec) { initStatus.setStatus(ec); });
  ASSERT_TRUE(initStatus.wait());

  ASSERT_THROW(newNode.getLastLocalBlockTimestamp(), std::exception);
}

TEST_F(InProcessNodeTests, getPoolDiffereceNotInited) {
  cryptonote::InProcessNode newNode(coreStub, protocolQueryStub);

  std::vector<crypto::hash_t> knownPoolTxIds; 
  crypto::hash_t knownBlockId = boost::value_initialized<crypto::hash_t>();
  bool isBcActual = false;
  std::vector<std::unique_ptr<ITransactionReader>> newTxs;
  std::vector<crypto::hash_t> deletedTxIds;

  CallbackStatus status;
  newNode.getPoolSymmetricDifference(std::move(knownPoolTxIds), knownBlockId, isBcActual, newTxs, deletedTxIds, [&status](std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_NE(std::error_code(), status.getStatus());
}

TEST_F(InProcessNodeTests, getPoolDiffereceActualBC) {
  size_t POOL_TX_NUMBER = 10;

  std::unordered_set<crypto::hash_t> transactionHashes;

  coreStub.setPoolChangesResult(true);

  for (size_t i = 0; i < POOL_TX_NUMBER; ++i) {
    auto txptr = cryptonote::createTransaction();
    auto tx = ::createTx(*txptr.get());
    transactionHashes.insert(cryptonote::getObjectHash(tx));
    cryptonote::tx_verification_context_t tvc = boost::value_initialized<tx_verification_context_t>();
    bool keptByBlock = false;
    coreStub.handleIncomingTransaction(tx, cryptonote::getObjectHash(tx), cryptonote::getObjectBinarySize(tx), tvc, keptByBlock);
    ASSERT_TRUE(tvc.m_added_to_pool);
    ASSERT_FALSE(tvc.m_verifivation_failed);
  }

  ASSERT_EQ(transactionHashes.size(), POOL_TX_NUMBER);

  std::vector<crypto::hash_t> knownPoolTxIds;
  crypto::hash_t knownBlockId = cryptonote::getObjectHash(generator.getBlockchain().back());
  bool isBcActual = false;
  std::vector<std::unique_ptr<ITransactionReader>> newTxs;
  std::vector<crypto::hash_t> deletedTxIds;

  CallbackStatus status;
  node.getPoolSymmetricDifference(std::move(knownPoolTxIds), knownBlockId, isBcActual, newTxs, deletedTxIds, [&status](std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_EQ(std::error_code(), status.getStatus());
  ASSERT_TRUE(isBcActual);
  ASSERT_EQ(newTxs.size(), transactionHashes.size());
  ASSERT_TRUE(deletedTxIds.empty());

  for (const auto& tx : newTxs) {
    ASSERT_NE(transactionHashes.find(tx->getTransactionHash()), transactionHashes.end());
  }
}

TEST_F(InProcessNodeTests, getPoolDiffereceNotActualBC) {
  size_t POOL_TX_NUMBER = 10;

  std::unordered_set<crypto::hash_t> transactionHashes;

  coreStub.setPoolChangesResult(false);

  for (size_t i = 0; i < POOL_TX_NUMBER; ++i) {
    auto txptr = cryptonote::createTransaction();
    auto tx = ::createTx(*txptr.get());
    transactionHashes.insert(cryptonote::getObjectHash(tx));
    cryptonote::tx_verification_context_t tvc = boost::value_initialized<tx_verification_context_t>();
    bool keptByBlock = false;
    coreStub.handleIncomingTransaction(tx, cryptonote::getObjectHash(tx), cryptonote::getObjectBinarySize(tx), tvc, keptByBlock);
    ASSERT_TRUE(tvc.m_added_to_pool);
    ASSERT_FALSE(tvc.m_verifivation_failed);
  }

  ASSERT_EQ(transactionHashes.size(), POOL_TX_NUMBER);

  std::vector<crypto::hash_t> knownPoolTxIds;
  crypto::hash_t knownBlockId = cryptonote::getObjectHash(generator.getBlockchain().back());
  bool isBcActual = false;
  std::vector<std::unique_ptr<ITransactionReader>> newTxs;
  std::vector<crypto::hash_t> deletedTxIds;

  CallbackStatus status;
  node.getPoolSymmetricDifference(std::move(knownPoolTxIds), knownBlockId, isBcActual, newTxs, deletedTxIds, [&status](std::error_code ec) { status.setStatus(ec); });
  ASSERT_TRUE(status.wait());
  ASSERT_EQ(std::error_code(), status.getStatus());
  ASSERT_FALSE(isBcActual);
  ASSERT_EQ(newTxs.size(), transactionHashes.size());
  ASSERT_TRUE(deletedTxIds.empty());

  for (const auto& tx : newTxs) {
    ASSERT_NE(transactionHashes.find(tx->getTransactionHash()), transactionHashes.end());
  }
}

//TODO: make relayTransaction unit test
//TODO: make getNewBlocks unit test
//TODO: make queryBlocks unit test
