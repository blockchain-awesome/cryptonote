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
#include "cryptonote/core/blockchain/serializer/crypto.h"

#include "cryptonote/core/key.h"
#include "cryptonote/core/currency.h"
#include "cryptonote/core/ITransactionValidator.h"
#include "cryptonote/core/ITxPoolObserver.h"
#include "cryptonote/core/VerificationContext.h"
#include "cryptonote/core/blockchain/indexing/exports.h"
#include "cryptonote/core/transaction/structures.h"

#include <logging/LoggerRef.h>

#include "cryptonote/core/transaction/once_in_time_interval.hpp"

namespace cryptonote {

  class ISerializer;

  using cryptonote::block_info_t;
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

    bool have_tx(const crypto::hash_t &id) const;
    bool add_tx(const transaction_t &tx, const crypto::hash_t &id, size_t blobSize, tx_verification_context_t& tvc, bool keeped_by_block);
    bool add_tx(const transaction_t &tx, tx_verification_context_t& tvc, bool keeped_by_block);
    //gets tx and remove it from pool
    bool take_tx(const crypto::hash_t &id, transaction_t &tx, size_t& blobSize, uint64_t& fee);

    bool on_blockchain_inc(uint64_t new_block_height, const crypto::hash_t& top_block_id);
    bool on_blockchain_dec(uint64_t new_block_height, const crypto::hash_t& top_block_id);

    void lock() const;
    void unlock() const;
    std::unique_lock<std::recursive_mutex> obtainGuard() const;

    bool fill_block_template(block_t &bl, size_t median_size, size_t maxCumulativeSize, uint64_t already_generated_coins, size_t &total_size, uint64_t &fee);

    void get_transactions(std::list<transaction_t>& txs) const;
    void get_difference(const std::vector<crypto::hash_t>& known_tx_ids, std::vector<crypto::hash_t>& new_tx_ids, std::vector<crypto::hash_t>& deleted_tx_ids) const;
    size_t get_transactions_count() const;
    std::string print_pool(bool short_format) const;
    void on_idle();

    bool getTransactionIdsByPaymentId(const crypto::hash_t& paymentId, std::vector<crypto::hash_t>& transactionIds);
    bool getTransactionIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<crypto::hash_t>& hashes, uint64_t& transactionsNumberWithinTimestamps);

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

    typedef hashed_unique<BOOST_MULTI_INDEX_MEMBER(transaction::transaction_details_t, crypto::hash_t, id)> main_index_t;
    typedef ordered_non_unique<identity<transaction::transaction_details_t>, transaction::transaction_priority_comparator_t> fee_index_t;

    typedef multi_index_container<transaction::transaction_details_t,
      indexed_by<main_index_t, fee_index_t>
    > tx_container_t;

    typedef std::pair<uint64_t, uint64_t> global_output_t;
    typedef std::set<global_output_t> global_output_container_t;
    typedef std::unordered_map<crypto::key_image_t, std::unordered_set<crypto::hash_t> > key_images_container_t;


    // double spending checking
    bool addTransactionInputs(const crypto::hash_t& id, const transaction_t& tx, bool keptByBlock);
    bool haveSpentInputs(const transaction_t& tx) const;
    bool removeTransactionInputs(const crypto::hash_t& id, const transaction_t& tx, bool keptByBlock);

    tx_container_t::iterator removeTransaction(tx_container_t::iterator i);
    bool removeExpiredTransactions();
    bool is_transaction_ready_to_go(const transaction_t& tx, transaction::transaction_check_info_t& txd) const;

    void buildIndices();

    Tools::ObserverManager<ITxPoolObserver> m_observerManager;
    const cryptonote::Currency& m_currency;
    OnceInTimeInterval m_txCheckInterval;
    mutable std::recursive_mutex m_transactions_lock;
    key_images_container_t m_spent_key_images;
    global_output_container_t m_spentOutputs;

    // std::string m_config_folder;
    cryptonote::ITransactionValidator& m_validator;
    cryptonote::ITimeProvider& m_timeProvider;

    tx_container_t m_transactions;  
    tx_container_t::nth_index<1>::type& m_fee_index;
    std::unordered_map<crypto::hash_t, uint64_t> m_recentlyDeletedTransactions;

    Logging::LoggerRef logger;

    PaymentIdIndex m_paymentIdIndex;
    TimestampTransactionsIndex m_timestampIndex;
  };
}


