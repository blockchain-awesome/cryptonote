// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <set>
#include <unordered_map>
#include <unordered_set>

#include <boost/utility.hpp>

// multi index
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include "common/int-util.h"
#include "common/ObserverManager.h"
#include "crypto/hash.h"
#include "crypto/crypto.h"
#include "cryptonote/core/serialize.h"

#include "cryptonote/core/key.h"
#include "cryptonote/core/Currency.h"
#include "cryptonote/core/ITransactionValidator.h"
#include "cryptonote/core/ITxPoolObserver.h"
#include "cryptonote/core/VerificationContext.h"
#include "cryptonote/core/blockchain/indexing/exports.h"
#include "cryptonote/core/transaction/structures.h"

#include <logging/LoggerRef.h>

#include "cryptonote/core/transaction/once_in_time_interval.hpp"

namespace cryptonote {

  class ISerializer;

  using cryptonote::BlockInfo;
  using namespace boost::multi_index;

  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  class TxMemoryPool: boost::noncopyable {
  public:
    TxMemoryPool(
      const cryptonote::Currency& currency, 
      cryptonote::ITransactionValidator& validator,
      cryptonote::ITimeProvider& timeProvider,
      Logging::ILogger& log);

    bool addObserver(ITxPoolObserver* observer);
    bool removeObserver(ITxPoolObserver* observer);

    // load/store operations
    bool init();
    bool deinit();

    bool have_tx(const crypto::Hash &id) const;
    bool add_tx(const Transaction &tx, const crypto::Hash &id, size_t blobSize, TxVerificationContext& tvc, bool keeped_by_block);
    bool add_tx(const Transaction &tx, TxVerificationContext& tvc, bool keeped_by_block);
    //gets tx and remove it from pool
    bool take_tx(const crypto::Hash &id, Transaction &tx, size_t& blobSize, uint64_t& fee);

    bool on_blockchain_inc(uint64_t new_block_height, const crypto::Hash& top_block_id);
    bool on_blockchain_dec(uint64_t new_block_height, const crypto::Hash& top_block_id);

    void lock() const;
    void unlock() const;
    std::unique_lock<std::recursive_mutex> obtainGuard() const;

    bool fill_block_template(block_t &bl, size_t median_size, size_t maxCumulativeSize, uint64_t already_generated_coins, size_t &total_size, uint64_t &fee);

    void get_transactions(std::list<Transaction>& txs) const;
    void get_difference(const std::vector<crypto::Hash>& known_tx_ids, std::vector<crypto::Hash>& new_tx_ids, std::vector<crypto::Hash>& deleted_tx_ids) const;
    size_t get_transactions_count() const;
    std::string print_pool(bool short_format) const;
    void on_idle();

    bool getTransactionIdsByPaymentId(const crypto::Hash& paymentId, std::vector<crypto::Hash>& transactionIds);
    bool getTransactionIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<crypto::Hash>& hashes, uint64_t& transactionsNumberWithinTimestamps);

    template<class t_ids_container, class t_tx_container, class t_missed_container>
    void getTransactions(const t_ids_container& txsIds, t_tx_container& txs, t_missed_container& missedTxs) {
      std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);

      for (const auto& id : txsIds) {
        auto it = m_transactions.find(id);
        if (it == m_transactions.end()) {
          missedTxs.push_back(id);
        } else {
          txs.push_back(it->tx);
        }
      }
    }

    void serialize(ISerializer& s);

  private:

    typedef hashed_unique<BOOST_MULTI_INDEX_MEMBER(transaction::TransactionDetails, crypto::Hash, id)> main_index_t;
    typedef ordered_non_unique<identity<transaction::TransactionDetails>, transaction::TransactionPriorityComparator> fee_index_t;

    typedef multi_index_container<transaction::TransactionDetails,
      indexed_by<main_index_t, fee_index_t>
    > tx_container_t;

    typedef std::pair<uint64_t, uint64_t> GlobalOutput;
    typedef std::set<GlobalOutput> GlobalOutputsContainer;
    typedef std::unordered_map<crypto::KeyImage, std::unordered_set<crypto::Hash> > key_images_container;


    // double spending checking
    bool addTransactionInputs(const crypto::Hash& id, const Transaction& tx, bool keptByBlock);
    bool haveSpentInputs(const Transaction& tx) const;
    bool removeTransactionInputs(const crypto::Hash& id, const Transaction& tx, bool keptByBlock);

    tx_container_t::iterator removeTransaction(tx_container_t::iterator i);
    bool removeExpiredTransactions();
    bool is_transaction_ready_to_go(const Transaction& tx, transaction::TransactionCheckInfo& txd) const;

    void buildIndices();

    Tools::ObserverManager<ITxPoolObserver> m_observerManager;
    const cryptonote::Currency& m_currency;
    OnceInTimeInterval m_txCheckInterval;
    mutable std::recursive_mutex m_transactions_lock;
    key_images_container m_spent_key_images;
    GlobalOutputsContainer m_spentOutputs;

    // std::string m_config_folder;
    cryptonote::ITransactionValidator& m_validator;
    cryptonote::ITimeProvider& m_timeProvider;

    tx_container_t m_transactions;  
    tx_container_t::nth_index<1>::type& m_fee_index;
    std::unordered_map<crypto::Hash, uint64_t> m_recentlyDeletedTransactions;

    Logging::LoggerRef logger;

    PaymentIdIndex m_paymentIdIndex;
    TimestampTransactionsIndex m_timestampIndex;
  };
}


