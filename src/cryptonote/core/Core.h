// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "p2p/NetNodeCommon.h"
#include "cryptonote/protocol/handler_common.h"
#include "Currency.h"
#include "tx_memory_pool.h"
#include "Blockchain.h"
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
  class LockedBlockchainStorage;
  struct CoreStateInfo;
  class miner;
  class CoreConfig;

  class core : public ICore, public IMinerHandler, public IBlockchainStorageObserver, public ITxPoolObserver {
   public:
     core(const Currency& currency, ICryptonoteProtocol* pprotocol, Logging::ILogger& logger);
     ~core();

     bool on_idle() override;
     virtual bool handle_incoming_tx(const BinaryArray& tx_blob, TxVerificationContext& tvc, bool keeped_by_block) override; //Deprecated. Should be removed with CryptoNoteProtocolHandler.
     bool handle_incoming_block_blob(const BinaryArray& block_blob, BlockVerificationContext& bvc, bool control_miner, bool relay_block) override;
     virtual ICryptonoteProtocol* get_protocol() override {return m_pprotocol;}
     const Currency& currency() const { return m_currency; }

     //-------------------- IMinerHandler -----------------------
     virtual bool handle_block_found(Block& b) override;
     virtual bool get_block_template(Block& b, const AccountPublicAddress& adr, difficulty_type& diffic, uint32_t& height, const BinaryArray& ex_nonce) override;

     bool addObserver(ICoreObserver* observer) override;
     bool removeObserver(ICoreObserver* observer) override;

     miner& get_miner() { return *m_miner; }
     bool init(const MinerConfig& minerConfig, bool load_existing);
     bool set_genesis_block(const Block& b);
     bool deinit();

     // ICore
     virtual size_t addChain(const std::vector<const IBlock*>& chain) override;
     virtual bool handle_get_objects(NOTIFY_REQUEST_GET_OBJECTS_request& arg, NOTIFY_RESPONSE_GET_OBJECTS_request& rsp) override; //Deprecated. Should be removed with CryptoNoteProtocolHandler.
     virtual bool getBackwardBlocksSizes(uint32_t fromHeight, std::vector<size_t>& sizes, size_t count) override;
     virtual bool getBlockSize(const crypto::Hash& hash, size_t& size) override;
     virtual bool getAlreadyGeneratedCoins(const crypto::Hash& hash, uint64_t& generatedCoins) override;
     virtual bool getBlockReward(size_t medianSize, size_t currentBlockSize, uint64_t alreadyGeneratedCoins, uint64_t fee,
                                 uint64_t& reward, int64_t& emissionChange) override;
     virtual bool scanOutputkeysForIndices(const KeyInput& txInToKey, std::list<std::pair<crypto::Hash, size_t>>& outputReferences) override;
     virtual bool getBlockDifficulty(uint32_t height, difficulty_type& difficulty) override;
     virtual bool getBlockContainingTx(const crypto::Hash& txId, crypto::Hash& blockId, uint32_t& blockHeight) override;
     virtual bool getMultisigOutputReference(const MultisignatureInput& txInMultisig, std::pair<crypto::Hash, size_t>& output_reference) override;
     virtual bool getGeneratedTransactionsNumber(uint32_t height, uint64_t& generatedTransactions) override;
     virtual bool getOrphanBlocksByHeight(uint32_t height, std::vector<Block>& blocks) override;
     virtual bool getBlocksByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<Block>& blocks, uint32_t& blocksNumberWithinTimestamps) override;
     virtual bool getPoolTransactionsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<Transaction>& transactions, uint64_t& transactionsNumberWithinTimestamps) override;
     virtual bool getTransactionsByPaymentId(const crypto::Hash& paymentId, std::vector<Transaction>& transactions) override;
     virtual bool getOutByMSigGIndex(uint64_t amount, uint64_t gindex, MultisignatureOutput& out) override;
     virtual std::unique_ptr<IBlock> getBlock(const crypto::Hash& blocksId) override;
     virtual bool handleIncomingTransaction(const Transaction& tx, const crypto::Hash& txHash, size_t blobSize, TxVerificationContext& tvc, bool keptByBlock) override;
     virtual std::error_code executeLocked(const std::function<std::error_code()>& func) override;
     
     virtual bool addMessageQueue(MessageQueue<BlockchainMessage>& messageQueue) override;
     virtual bool removeMessageQueue(MessageQueue<BlockchainMessage>& messageQueue) override;

     uint32_t get_current_blockchain_height();
     bool have_block(const crypto::Hash& id) override;
     std::vector<crypto::Hash> buildSparseChain() override;
     std::vector<crypto::Hash> buildSparseChain(const crypto::Hash& startBlockId) override;
     void on_synchronized() override;

     virtual void get_blockchain_top(uint32_t& height, crypto::Hash& top_id) override;
     bool get_blocks(uint32_t start_offset, uint32_t count, std::list<Block>& blocks, std::list<Transaction>& txs);
     bool get_blocks(uint32_t start_offset, uint32_t count, std::list<Block>& blocks);
     template<class t_ids_container, class t_blocks_container, class t_missed_container>
     bool get_blocks(const t_ids_container& block_ids, t_blocks_container& blocks, t_missed_container& missed_bs)
     {
       return m_blockchain.getBlocks(block_ids, blocks, missed_bs);
     }
     virtual bool queryBlocks(const std::vector<crypto::Hash>& block_ids, uint64_t timestamp,
       uint32_t& start_height, uint32_t& current_height, uint32_t& full_offset, std::vector<BlockFullInfo>& entries) override;
    virtual bool queryBlocksLite(const std::vector<crypto::Hash>& knownBlockIds, uint64_t timestamp,
      uint32_t& resStartHeight, uint32_t& resCurrentHeight, uint32_t& resFullOffset, std::vector<BlockShortInfo>& entries) override;
    virtual crypto::Hash getBlockIdByHeight(uint32_t height) override;
     void getTransactions(const std::vector<crypto::Hash>& txs_ids, std::list<Transaction>& txs, std::list<crypto::Hash>& missed_txs, bool checkTxPool = false) override;
     virtual bool getBlockByHash(const crypto::Hash &h, Block &blk) override;
     virtual bool getBlockHeight(const crypto::Hash& blockId, uint32_t& blockHeight) override;
     //void get_all_known_block_ids(std::list<crypto::Hash> &main, std::list<crypto::Hash> &alt, std::list<crypto::Hash> &invalid);

     bool get_alternative_blocks(std::list<Block>& blocks);
     size_t get_alternative_blocks_count();

     void set_cryptonote_protocol(ICryptonoteProtocol* pprotocol);
     void set_checkpoints(Checkpoints&& chk_pts);

     std::vector<Transaction> getPoolTransactions() override;
     size_t get_pool_transactions_count();
     size_t get_blockchain_total_transactions();
     //bool get_outs(uint64_t amount, std::list<crypto::PublicKey>& pkeys);
     virtual std::vector<crypto::Hash> findBlockchainSupplement(const std::vector<crypto::Hash>& remoteBlockIds, size_t maxCount,
       uint32_t& totalBlockCount, uint32_t& startBlockIndex) override;
     bool get_stat_info(CoreStateInfo& st_inf) override;
     
     virtual bool get_tx_outputs_gindexs(const crypto::Hash& tx_id, std::vector<uint32_t>& indexs) override;
     crypto::Hash get_tail_id();
     virtual bool get_random_outs_for_amounts(const COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_request& req, COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS_response& res) override;
     void pause_mining() override;
     void update_block_template_and_resume_mining() override;
     //Blockchain& get_blockchain_storage(){return m_blockchain;}
     //debug functions
     void print_blockchain(uint32_t start_index, uint32_t end_index);
     void print_blockchain_index();
     std::string print_pool(bool short_format);
     void print_blockchain_outs(const std::string& file);
     virtual bool getPoolChanges(const crypto::Hash& tailBlockId, const std::vector<crypto::Hash>& knownTxsIds,
                                 std::vector<Transaction>& addedTxs, std::vector<crypto::Hash>& deletedTxsIds) override;
     virtual bool getPoolChangesLite(const crypto::Hash& tailBlockId, const std::vector<crypto::Hash>& knownTxsIds,
                                  std::vector<TransactionPrefixInfo>& addedTxs, std::vector<crypto::Hash>& deletedTxsIds) override;
     virtual void getPoolChanges(const std::vector<crypto::Hash>& knownTxsIds, std::vector<Transaction>& addedTxs,
                                 std::vector<crypto::Hash>& deletedTxsIds) override;

     uint64_t getNextBlockDifficulty();
     uint64_t getTotalGeneratedAmount();

   private:
     bool add_new_tx(const Transaction& tx, const crypto::Hash& tx_hash, size_t blob_size, TxVerificationContext& tvc, bool keeped_by_block);
     bool load_state_data();
     bool parse_tx_from_blob(Transaction& tx, crypto::Hash& tx_hash, crypto::Hash& tx_prefix_hash, const BinaryArray& blob);
     bool handle_incoming_block(const Block& b, BlockVerificationContext& bvc, bool control_miner, bool relay_block);

     bool check_tx_syntax(const Transaction& tx);
     //check correct values, amounts and all lightweight checks not related with database
     bool check_tx_semantic(const Transaction& tx, bool keeped_by_block);
     //check if tx already in memory pool or in main blockchain

     bool is_key_image_spent(const crypto::KeyImage& key_im);

     bool check_tx_ring_signature(const KeyInput& tx, const crypto::Hash& tx_prefix_hash, const std::vector<crypto::Signature>& sig);
     bool is_tx_spendtime_unlocked(uint64_t unlock_time);
     bool update_miner_block_template();
     bool on_update_blocktemplate_interval();
     bool check_tx_inputs_keyimages_diff(const Transaction& tx);
     virtual void blockchainUpdated() override;
     virtual void txDeletedFromPool() override;
     void poolUpdated();

     bool findStartAndFullOffsets(const std::vector<crypto::Hash>& knownBlockIds, uint64_t timestamp, uint32_t& startOffset, uint32_t& startFullOffset);
     std::vector<crypto::Hash> findIdsForShortBlocks(uint32_t startOffset, uint32_t startFullOffset);

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
