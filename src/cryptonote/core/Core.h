// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "p2p/NetNodeCommon.h"
#include "cryptonote/protocol/handler_common.h"
#include "currency.h"
#include "tx_memory_pool.h"
#include "blockchain.h"
#include "cryptonote/core/IMinerHandler.h"
#include "command_line/MinerConfig.h"
#include "ICore.h"
#include "ICoreObserver.h"
#include "common/ObserverManager.h"

#include "system/Dispatcher.h"
#include "cryptonote/core/template/MessageQueue.h"
#include "cryptonote/core/BlockchainMessages.h"

#include <logging/LoggerMessage.h>

namespace cryptonote {
  class Locker;
  struct CoreStateInfo;
  class miner;
  class CoreConfig;

  class core : public ICore, public IMinerHandler, public IBlockchainStorageObserver, public ITxPoolObserver {
   public:
     core(const Currency& currency, ICryptonoteProtocol* pprotocol, Logging::ILogger& logger);
     ~core();

     bool on_idle() override;
     virtual bool handle_incoming_tx(const binary_array_t& tx_blob, tx_verification_context_t& tvc, bool keeped_by_block) override; //Deprecated. Should be removed with CryptoNoteProtocolHandler.
     bool handle_incoming_block_blob(const binary_array_t& block_blob, block_verification_context_t& bvc, bool control_miner, bool relay_block) override;
     virtual ICryptonoteProtocol* get_protocol() override {return m_pprotocol;}
     const Currency& currency() const { return m_currency; }

     //-------------------- IMinerHandler -----------------------
     virtual bool handle_block_found(block_t& b) override;
     virtual bool get_block_template(block_t& b, const account_public_address_t& adr, difficulty_t& diffic, uint32_t& height, const binary_array_t& ex_nonce) override;

     bool addObserver(ICoreObserver* observer) override;
     bool removeObserver(ICoreObserver* observer) override;

     miner& get_miner() { return *m_miner; }
     bool init(const MinerConfig& minerConfig, bool load_existing);
     bool set_genesis_block(const block_t& b);
     bool deinit();

     // ICore
     virtual size_t addChain(const std::vector<const IBlock*>& chain) override;
     virtual bool handle_get_objects(NOTIFY_REQUEST_GET_OBJECTS_request& arg, NOTIFY_RESPONSE_GET_OBJECTS_request& rsp) override; //Deprecated. Should be removed with CryptoNoteProtocolHandler.
     virtual bool getBackwardBlocksSizes(uint32_t fromHeight, std::vector<size_t>& sizes, size_t count) override;
     virtual bool getBlockSize(const crypto::hash_t& hash, size_t& size) override;
     virtual bool getAlreadyGeneratedCoins(const crypto::hash_t& hash, uint64_t& generatedCoins) override;
     virtual bool getBlockReward(size_t medianSize, size_t currentBlockSize, uint64_t alreadyGeneratedCoins, uint64_t fee,
                                 uint64_t& reward, int64_t& emissionChange) override;
     virtual bool scanOutputkeysForIndices(const key_input_t& txInToKey, std::list<std::pair<crypto::hash_t, size_t>>& outputReferences) override;
     virtual bool getBlockDifficulty(uint32_t height, difficulty_t& difficulty) override;
     virtual bool getBlockContainingTx(const crypto::hash_t& txId, crypto::hash_t& blockId, uint32_t& blockHeight) override;
     virtual bool getMultisigOutputReference(const multi_signature_input_t& txInMultisig, std::pair<crypto::hash_t, size_t>& output_reference) override;
     virtual bool getGeneratedTransactionsNumber(uint32_t height, uint64_t& generatedTransactions) override;
     virtual bool getOrphanBlocksByHeight(uint32_t height, std::vector<block_t>& blocks) override;
     virtual bool getBlocksByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<block_t>& blocks, uint32_t& blocksNumberWithinTimestamps) override;
     virtual bool getPoolTransactionsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<transaction_t>& transactions, uint64_t& transactionsNumberWithinTimestamps) override;
     virtual bool getTransactionsByPaymentId(const crypto::hash_t& paymentId, std::vector<transaction_t>& transactions) override;
     virtual bool getOutByMSigGIndex(uint64_t amount, uint64_t gindex, multi_signature_output_t& out) override;
     virtual std::unique_ptr<IBlock> getBlock(const crypto::hash_t& blocksId) override;
     virtual bool handleIncomingTransaction(const transaction_t& tx, const crypto::hash_t& txHash, size_t blobSize, tx_verification_context_t& tvc, bool keptByBlock) override;
     virtual std::error_code executeLocked(const std::function<std::error_code()>& func) override;
     
     virtual bool addMessageQueue(MessageQueue<BlockchainMessage>& messageQueue) override;
     virtual bool removeMessageQueue(MessageQueue<BlockchainMessage>& messageQueue) override;

     uint32_t get_current_blockchain_height();
     bool have_block(const crypto::hash_t& id) override;
     std::vector<crypto::hash_t> buildSparseChain() override;
     std::vector<crypto::hash_t> buildSparseChain(const crypto::hash_t& startBlockId) override;
     void on_synchronized() override;

     virtual void get_blockchain_top(uint32_t& height, crypto::hash_t& top_id) override;
     bool get_blocks(uint32_t start_offset, uint32_t count, std::list<block_t>& blocks, std::list<transaction_t>& txs);
     bool get_blocks(uint32_t start_offset, uint32_t count, std::list<block_t>& blocks);
     template<class t_ids_container, class t_blocks_container, class t_missed_container>
     bool get_blocks(const t_ids_container& block_ids, t_blocks_container& blocks, t_missed_container& missed_bs)
     {
       return m_blockchain.getBlocks(block_ids, blocks, missed_bs);
     }
     virtual bool queryBlocks(const std::vector<crypto::hash_t>& block_ids, uint64_t timestamp,
       uint32_t& start_height, uint32_t& current_height, uint32_t& full_offset, std::vector<block_full_info_t>& entries) override;
    virtual bool queryBlocksLite(const std::vector<crypto::hash_t>& knownBlockIds, uint64_t timestamp,
      uint32_t& resStartHeight, uint32_t& resCurrentHeight, uint32_t& resFullOffset, std::vector<block_short_info_t>& entries) override;
    virtual crypto::hash_t getBlockIdByHeight(uint32_t height) override;
    void getTransactions(const std::vector<crypto::hash_t>& txs_ids, std::list<transaction_t>& txs, std::list<crypto::hash_t>& missed_txs, bool checkTxPool = false) override;
    virtual bool getBlockByHash(const crypto::hash_t &h, block_t &blk) override;

     bool get_alternative_blocks(std::list<block_t>& blocks);
     size_t get_alternative_blocks_count();

     void set_cryptonote_protocol(ICryptonoteProtocol* pprotocol);
     void set_checkpoints(Checkpoints&& chk_pts);

     std::vector<transaction_t> getPoolTransactions() override;
     size_t get_pool_transactions_count();
     size_t get_blockchain_total_transactions();
     //bool get_outs(uint64_t amount, std::list<crypto::public_key_t>& pkeys);
     virtual std::vector<crypto::hash_t> findBlockchainSupplement(const std::vector<crypto::hash_t>& remoteBlockIds, size_t maxCount,
       uint32_t& totalBlockCount, uint32_t& startBlockIndex) override;
     bool get_stat_info(CoreStateInfo& st_inf) override;
     
     virtual bool get_tx_outputs_gindexs(const crypto::hash_t& tx_id, std::vector<uint32_t>& indexs) override;
     crypto::hash_t get_tail_id();
     virtual bool get_random_outs_for_amounts(const COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_request& req, COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_response& res) override;
     void pause_mining() override;
     void update_block_template_and_resume_mining() override;
     //Blockchain& get_blockchain_storage(){return m_blockchain;}
     //debug functions
     void print_blockchain(uint32_t start_index, uint32_t end_index);
     void print_blockchain_index();
     std::string print_pool(bool short_format);
     void print_blockchain_outs(const std::string& file);
     virtual bool getPoolChanges(const crypto::hash_t& tailBlockId, const std::vector<crypto::hash_t>& knownTxsIds,
                                 std::vector<transaction_t>& addedTxs, std::vector<crypto::hash_t>& deletedTxsIds) override;
     virtual bool getPoolChangesLite(const crypto::hash_t& tailBlockId, const std::vector<crypto::hash_t>& knownTxsIds,
                                  std::vector<transaction_prefix_info_t>& addedTxs, std::vector<crypto::hash_t>& deletedTxsIds) override;
     virtual void getPoolChanges(const std::vector<crypto::hash_t>& knownTxsIds, std::vector<transaction_t>& addedTxs,
                                 std::vector<crypto::hash_t>& deletedTxsIds) override;

     uint64_t getNextBlockDifficulty();
     uint64_t getTotalGeneratedAmount();

   private:
     bool add_new_tx(const transaction_t& tx, const crypto::hash_t& tx_hash, size_t blob_size, tx_verification_context_t& tvc, bool keeped_by_block);
     bool load_state_data();
     bool parse_tx_from_blob(transaction_t& tx, crypto::hash_t& tx_hash, crypto::hash_t& tx_prefix_hash, const binary_array_t& blob);
     bool handle_incoming_block(const block_t& b, block_verification_context_t& bvc, bool control_miner, bool relay_block);

     bool check_tx_syntax(const transaction_t& tx);
     //check correct values, amounts and all lightweight checks not related with database
     bool check_tx_semantic(const transaction_t& tx, bool keeped_by_block);
     //check if tx already in memory pool or in main blockchain

     bool is_key_image_spent(const crypto::key_image_t& key_im);

     bool check_tx_ring_signature(const key_input_t& tx, const crypto::hash_t& tx_prefix_hash, const std::vector<crypto::signature_t>& sig);
     bool is_tx_spendtime_unlocked(uint64_t unlock_time);
     bool update_miner_block_template();
     bool on_update_blocktemplate_interval();
     bool check_tx_inputs_keyimages_diff(const transaction_t& tx);
     virtual void blockchainUpdated() override;
     virtual void txDeletedFromPool() override;
     void poolUpdated();

     bool findStartAndFullOffsets(const std::vector<crypto::hash_t>& knownBlockIds, uint64_t timestamp, uint32_t& startOffset, uint32_t& startFullOffset);
     std::vector<crypto::hash_t> findIdsForShortBlocks(uint32_t startOffset, uint32_t startFullOffset);

     const Currency& m_currency;
     Logging::LoggerRef logger;
     cryptonote::RealTimeProvider m_timeProvider;
     TxMemoryPool m_mempool;
     Blockchain m_blockchain;
     ICryptonoteProtocol* m_pprotocol;
     std::unique_ptr<miner> m_miner;
    //  std::string m_config_folder;
     cryptonote_protocol_stub m_protocol_stub;
     friend class tx_validate_inputs;
     std::atomic<bool> m_starter_message_showed;
     Tools::ObserverManager<ICoreObserver> m_observerManager;
   };
}
