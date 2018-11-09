// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <unordered_map>

#include "cryptonote/core/key.h"
#include "cryptonote/core/ICore.h"
#include "cryptonote/core/ICoreObserver.h"
#include "cryptonote/protocol/definitions.h"
#include "rpc/CoreRpcServerCommandsDefinitions.h"

class ICoreStub: public cryptonote::ICore {
public:
  ICoreStub();
  ICoreStub(const cryptonote::block_t& genesisBlock);

  virtual bool addObserver(cryptonote::ICoreObserver* observer) override;
  virtual bool removeObserver(cryptonote::ICoreObserver* observer) override;
  virtual void get_blockchain_top(uint32_t& height, crypto::hash_t& top_id) override;
  virtual std::vector<crypto::hash_t> findBlockchainSupplement(const std::vector<crypto::hash_t>& remoteBlockIds, size_t maxCount,
    uint32_t& totalBlockCount, uint32_t& startBlockIndex) override;
  virtual bool get_random_outs_for_amounts(const cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_request& req,
      cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_response& res) override;
  virtual bool get_tx_outputs_gindexs(const crypto::hash_t& tx_id, std::vector<uint32_t>& indexs) override;
  virtual cryptonote::ICryptonoteProtocol* get_protocol() override;
  virtual bool handle_incoming_tx(cryptonote::binary_array_t const& tx_blob, cryptonote::tx_verification_context_t& tvc, bool keeped_by_block) override;
  virtual std::vector<cryptonote::transaction_t> getPoolTransactions() override;
  virtual bool getPoolChanges(const crypto::hash_t& tailBlockId, const std::vector<crypto::hash_t>& knownTxsIds,
                              std::vector<cryptonote::transaction_t>& addedTxs, std::vector<crypto::hash_t>& deletedTxsIds) override;
  virtual bool getPoolChangesLite(const crypto::hash_t& tailBlockId, const std::vector<crypto::hash_t>& knownTxsIds,
          std::vector<cryptonote::transaction_prefix_info_t>& addedTxs, std::vector<crypto::hash_t>& deletedTxsIds) override;
  virtual void getPoolChanges(const std::vector<crypto::hash_t>& knownTxsIds, std::vector<cryptonote::transaction_t>& addedTxs,
                              std::vector<crypto::hash_t>& deletedTxsIds) override;
  virtual bool queryBlocks(const std::vector<crypto::hash_t>& block_ids, uint64_t timestamp,
    uint32_t& start_height, uint32_t& current_height, uint32_t& full_offset, std::vector<cryptonote::block_full_info_t>& entries) override;
  virtual bool queryBlocksLite(const std::vector<crypto::hash_t>& block_ids, uint64_t timestamp,
    uint32_t& start_height, uint32_t& current_height, uint32_t& full_offset, std::vector<cryptonote::block_short_info_t>& entries) override;

  virtual bool have_block(const crypto::hash_t& id) override;
  std::vector<crypto::hash_t> buildSparseChain() override;
  std::vector<crypto::hash_t> buildSparseChain(const crypto::hash_t& startBlockId) override;
  virtual bool get_stat_info(cryptonote::CoreStateInfo& st_inf) override { return false; }
  virtual bool on_idle() override { return false; }
  virtual void pause_mining() override {}
  virtual void update_block_template_and_resume_mining() override {}
  virtual bool handle_incoming_block_blob(const cryptonote::binary_array_t& block_blob, cryptonote::block_verification_context_t& bvc, bool control_miner, bool relay_block) override { return false; }
  virtual bool handle_get_objects(cryptonote::NOTIFY_REQUEST_GET_OBJECTS::request& arg, cryptonote::NOTIFY_RESPONSE_GET_OBJECTS::request& rsp) override { return false; }
  virtual void on_synchronized() override {}
  virtual bool getOutByMSigGIndex(uint64_t amount, uint64_t gindex, cryptonote::multi_signature_output_t& out) override { return true; }
  virtual size_t addChain(const std::vector<const cryptonote::IBlock*>& chain) override;

  virtual crypto::hash_t getBlockIdByHeight(uint32_t height) override;
  virtual bool getBlockByHash(const crypto::hash_t &h, cryptonote::block_t &blk) override;
  virtual bool getBlockHeight(const crypto::hash_t& blockId, uint32_t& blockHeight) override;
  virtual void getTransactions(const std::vector<crypto::hash_t>& txs_ids, std::list<cryptonote::transaction_t>& txs, std::list<crypto::hash_t>& missed_txs, bool checkTxPool = false) override;
  virtual bool getBackwardBlocksSizes(uint32_t fromHeight, std::vector<size_t>& sizes, size_t count) override;
  virtual bool getBlockSize(const crypto::hash_t& hash, size_t& size) override;
  virtual bool getAlreadyGeneratedCoins(const crypto::hash_t& hash, uint64_t& generatedCoins) override;
  virtual bool getBlockReward(size_t medianSize, size_t currentBlockSize, uint64_t alreadyGeneratedCoins, uint64_t fee,
      uint64_t& reward, int64_t& emissionChange) override;
  virtual bool scanOutputkeysForIndices(const cryptonote::key_input_t& txInToKey, std::list<std::pair<crypto::hash_t, size_t>>& outputReferences) override;
  virtual bool getBlockDifficulty(uint32_t height, cryptonote::difficulty_t& difficulty) override;
  virtual bool getBlockContainingTx(const crypto::hash_t& txId, crypto::hash_t& blockId, uint32_t& blockHeight) override;
  virtual bool getMultisigOutputReference(const cryptonote::multi_signature_input_t& txInMultisig, std::pair<crypto::hash_t, size_t>& outputReference) override;

  virtual bool getGeneratedTransactionsNumber(uint32_t height, uint64_t& generatedTransactions) override;
  virtual bool getOrphanBlocksByHeight(uint32_t height, std::vector<cryptonote::block_t>& blocks) override;
  virtual bool getBlocksByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<cryptonote::block_t>& blocks, uint32_t& blocksNumberWithinTimestamps) override;
  virtual bool getPoolTransactionsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<cryptonote::transaction_t>& transactions, uint64_t& transactionsNumberWithinTimestamps) override;
  virtual bool getTransactionsByPaymentId(const crypto::hash_t& paymentId, std::vector<cryptonote::transaction_t>& transactions) override;
  virtual std::unique_ptr<cryptonote::IBlock> getBlock(const crypto::hash_t& blockId) override;
  virtual bool handleIncomingTransaction(const cryptonote::transaction_t& tx, const crypto::hash_t& txHash, size_t blobSize, cryptonote::tx_verification_context_t& tvc, bool keptByBlock) override;
  virtual std::error_code executeLocked(const std::function<std::error_code()>& func) override;

  virtual bool addMessageQueue(cryptonote::MessageQueue<cryptonote::BlockchainMessage>& messageQueuePtr) override;
  virtual bool removeMessageQueue(cryptonote::MessageQueue<cryptonote::BlockchainMessage>& messageQueuePtr) override;


  void set_blockchain_top(uint32_t height, const crypto::hash_t& top_id);
  void set_outputs_gindexs(const std::vector<uint32_t>& indexs, bool result);
  void set_random_outs(const cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_response& resp, bool result);

  void addBlock(const cryptonote::block_t& block);
  void addTransaction(const cryptonote::transaction_t& tx);

  void setPoolTxVerificationResult(bool result);
  void setPoolChangesResult(bool result);

private:
  uint32_t topHeight;
  crypto::hash_t topId;

  std::vector<uint32_t> globalIndices;
  bool globalIndicesResult;

  cryptonote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_response randomOuts;
  bool randomOutsResult;

  std::unordered_map<crypto::hash_t, cryptonote::block_t> blocks;
  std::unordered_map<uint32_t, crypto::hash_t> blockHashByHeightIndex;
  std::unordered_map<crypto::hash_t, crypto::hash_t> blockHashByTxHashIndex;

  std::unordered_map<crypto::hash_t, cryptonote::transaction_t> transactions;
  std::unordered_map<crypto::hash_t, cryptonote::transaction_t> transactionPool;
  bool poolTxVerificationResult;
  bool poolChangesResult;
};
