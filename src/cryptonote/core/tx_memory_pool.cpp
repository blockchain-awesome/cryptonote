// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tx_memory_pool.h"

#include <algorithm>
#include <ctime>
#include <vector>
#include <unordered_set>

#include <boost/filesystem.hpp>

#include "common/int-util.h"
#include "crypto/hash.h"

#include "serialization/SerializationTools.h"
#include "serialization/BinarySerializationTools.h"

#include "CryptoNoteFormatUtils.h"
#include "CryptoNoteTools.h"
#include "CryptoNoteConfig.h"

using namespace Logging;

#undef ERROR

namespace cryptonote {

  //---------------------------------------------------------------------------------
  // BlockTemplate
  //---------------------------------------------------------------------------------
  class BlockTemplate {
  public:

    bool addTransaction(const crypto::hash_t& txid, const transaction_t& tx) {
      if (!canAdd(tx))
        return false;

      for (const auto& in : tx.inputs) {
        if (in.type() == typeid(key_input_t)) {
          auto r = m_keyImages.insert(boost::get<key_input_t>(in).keyImage);
          (void)r; //just to make compiler to shut up
          assert(r.second);
        } else if (in.type() == typeid(multi_signature_input_t)) {
          const auto& msig = boost::get<multi_signature_input_t>(in);
          auto r = m_usedOutputs.insert(std::make_pair(msig.amount, msig.outputIndex));
          (void)r; //just to make compiler to shut up
          assert(r.second);
        }
      }

      m_txHashes.push_back(txid);
      return true;
    }

    const std::vector<crypto::hash_t>& getTransactions() const {
      return m_txHashes;
    }

  private:

    bool canAdd(const transaction_t& tx) {
      for (const auto& in : tx.inputs) {
        if (in.type() == typeid(key_input_t)) {
          if (m_keyImages.count(boost::get<key_input_t>(in).keyImage)) {
            return false;
          }
        } else if (in.type() == typeid(multi_signature_input_t)) {
          const auto& msig = boost::get<multi_signature_input_t>(in);
          if (m_usedOutputs.count(std::make_pair(msig.amount, msig.outputIndex))) {
            return false;
          }
        }
      }
      return true;
    }
    
    std::unordered_set<crypto::key_image_t> m_keyImages;
    std::set<std::pair<uint64_t, uint64_t>> m_usedOutputs;
    std::vector<crypto::hash_t> m_txHashes;
  };

  using cryptonote::block_info_t;

  //---------------------------------------------------------------------------------
  TxMemoryPool::TxMemoryPool(
    const cryptonote::Currency& currency, 
    cryptonote::ITransactionValidator& validator, 
    cryptonote::ITimeProvider& timeProvider,
    Logging::ILogger& log) :
    m_currency(currency),
    m_validator(validator), 
    m_timeProvider(timeProvider), 
    m_txCheckInterval(60, timeProvider),
    m_fee_index(boost::get<1>(m_transactions)),
    logger(log, "txpool") {
  }
  //---------------------------------------------------------------------------------
  bool TxMemoryPool::add_tx(const transaction_t &tx, /*const crypto::hash_t& tx_prefix_hash,*/ const crypto::hash_t &id, size_t blobSize, tx_verification_context_t& tvc, bool keptByBlock) {
    if (!check_inputs_types_supported(tx)) {
      tvc.m_verifivation_failed = true;
      return false;
    }

    uint64_t inputs_amount = 0;
    if (!get_inputs_money_amount(tx, inputs_amount)) {
      tvc.m_verifivation_failed = true;
      return false;
    }

    uint64_t outputs_amount = get_outs_money_amount(tx);

    if (outputs_amount > inputs_amount) {
      logger(INFO) << "transaction use more money then it has: use " << m_currency.formatAmount(outputs_amount) <<
        ", have " << m_currency.formatAmount(inputs_amount);
      tvc.m_verifivation_failed = true;
      return false;
    }

    const uint64_t fee = inputs_amount - outputs_amount;
    bool isFusionTransaction = fee == 0 && m_currency.isFusionTransaction(tx, blobSize);
    if (!keptByBlock && !isFusionTransaction && fee < m_currency.minimumFee()) {
      logger(INFO) << "transaction fee is not enough: " << m_currency.formatAmount(fee) <<
        ", minimum fee: " << m_currency.formatAmount(m_currency.minimumFee());
      tvc.m_verifivation_failed = true;
      tvc.m_tx_fee_too_small = true;
      return false;
    }

    //check key images for transaction if it is not kept by block
    if (!keptByBlock) {
      std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
      if (haveSpentInputs(tx)) {
        logger(INFO) << "transaction_t with id= " << id << " used already spent inputs";
        tvc.m_verifivation_failed = true;
        return false;
      }
    }

    block_info_t maxUsedBlock;

    // check inputs
    bool inputsValid = m_validator.checkTransactionInputs(tx, maxUsedBlock);

    if (!inputsValid) {
      if (!keptByBlock) {
        logger(INFO) << "tx used wrong inputs, rejected";
        tvc.m_verifivation_failed = true;
        return false;
      }

      maxUsedBlock.clear();
      tvc.m_verifivation_impossible = true;
    }

    if (!keptByBlock) {
      bool sizeValid = m_validator.checkTransactionSize(blobSize);
      if (!sizeValid) {
        logger(INFO) << "tx too big, rejected";
        tvc.m_verifivation_failed = true;
        return false;
      }
    }

    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);

    if (!keptByBlock && m_recentlyDeletedTransactions.find(id) != m_recentlyDeletedTransactions.end()) {
      logger(INFO) << "Trying to add recently deleted transaction. Ignore: " << id;
      tvc.m_verifivation_failed = false;
      tvc.m_should_be_relayed = false;
      tvc.m_added_to_pool = false;
      return true;
    }

    // add to pool
    {
      transaction::transaction_details_t txd;

      txd.id = id;
      txd.blobSize = blobSize;
      txd.tx = tx;
      txd.fee = fee;
      txd.keptByBlock = keptByBlock;
      txd.receiveTime = m_timeProvider.now();

      txd.maxUsedBlock = maxUsedBlock;
      txd.lastFailedBlock.clear();

      auto txd_p = m_transactions.insert(std::move(txd));
      if (!(txd_p.second)) {
        logger(ERROR, BRIGHT_RED) << "transaction already exists at inserting in memory pool";
        return false;
      }
      m_paymentIdIndex.add(txd.tx);
      m_timestampIndex.add(txd.receiveTime, txd.id);

    }

    tvc.m_added_to_pool = true;
    tvc.m_should_be_relayed = inputsValid && (fee > 0 || isFusionTransaction);
    tvc.m_verifivation_failed = true;

    if (!addTransactionInputs(id, tx, keptByBlock))
      return false;

    tvc.m_verifivation_failed = false;
    //succeed
    return true;
  }

  //---------------------------------------------------------------------------------
  bool TxMemoryPool::add_tx(const transaction_t &tx, tx_verification_context_t& tvc, bool keeped_by_block) {
    crypto::hash_t h = NULL_HASH;
    size_t blobSize = 0;
    getObjectHash(tx, h, blobSize);
    return add_tx(tx, h, blobSize, tvc, keeped_by_block);
  }
  //---------------------------------------------------------------------------------
  bool TxMemoryPool::take_tx(const crypto::hash_t &id, transaction_t &tx, size_t& blobSize, uint64_t& fee) {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    auto it = m_transactions.find(id);
    if (it == m_transactions.end()) {
      return false;
    }

    auto& txd = *it;

    tx = txd.tx;
    blobSize = txd.blobSize;
    fee = txd.fee;

    removeTransaction(it);
    return true;
  }
  //---------------------------------------------------------------------------------
  size_t TxMemoryPool::get_transactions_count() const {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    return m_transactions.size();
  }
  //---------------------------------------------------------------------------------
  void TxMemoryPool::get_transactions(std::list<transaction_t>& txs) const {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    for (const auto& tx_vt : m_transactions) {
      txs.push_back(tx_vt.tx);
    }
  }
  //---------------------------------------------------------------------------------
  void TxMemoryPool::get_difference(const std::vector<crypto::hash_t>& known_tx_ids, std::vector<crypto::hash_t>& new_tx_ids, std::vector<crypto::hash_t>& deleted_tx_ids) const {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    std::unordered_set<crypto::hash_t> ready_tx_ids;
    for (const auto& tx : m_transactions) {
      transaction::transaction_check_info_t checkInfo(tx);
      if (is_transaction_ready_to_go(tx.tx, checkInfo)) {
        ready_tx_ids.insert(tx.id);
      }
    }

    std::unordered_set<crypto::hash_t> known_set(known_tx_ids.begin(), known_tx_ids.end());
    for (auto it = ready_tx_ids.begin(), e = ready_tx_ids.end(); it != e;) {
      auto known_it = known_set.find(*it);
      if (known_it != known_set.end()) {
        known_set.erase(known_it);
        it = ready_tx_ids.erase(it);
      }
      else {
        ++it;
      }
    }

    new_tx_ids.assign(ready_tx_ids.begin(), ready_tx_ids.end());
    deleted_tx_ids.assign(known_set.begin(), known_set.end());
  }
  //---------------------------------------------------------------------------------
  bool TxMemoryPool::on_blockchain_inc(uint64_t new_block_height, const crypto::hash_t& top_block_id) {
    return true;
  }
  //---------------------------------------------------------------------------------
  bool TxMemoryPool::on_blockchain_dec(uint64_t new_block_height, const crypto::hash_t& top_block_id) {
    return true;
  }
  //---------------------------------------------------------------------------------
  bool TxMemoryPool::have_tx(const crypto::hash_t &id) const {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    if (m_transactions.count(id)) {
      return true;
    }
    return false;
  }
  //---------------------------------------------------------------------------------
  void TxMemoryPool::lock() const {
    m_transactions_lock.lock();
  }
  //---------------------------------------------------------------------------------
  void TxMemoryPool::unlock() const {
    m_transactions_lock.unlock();
  }

  std::unique_lock<std::recursive_mutex> TxMemoryPool::obtainGuard() const {
    return std::unique_lock<std::recursive_mutex>(m_transactions_lock);
  }

  //---------------------------------------------------------------------------------
  bool TxMemoryPool::is_transaction_ready_to_go(const transaction_t& tx, transaction::transaction_check_info_t& txd) const {

    if (!m_validator.checkTransactionInputs(tx, txd.maxUsedBlock, txd.lastFailedBlock))
      return false;

    //if we here, transaction seems valid, but, anyway, check for key_images collisions with blockchain, just to be sure
    if (m_validator.haveSpentKeyImages(tx))
      return false;

    //transaction is ok.
    return true;
  }
  //---------------------------------------------------------------------------------
  std::string TxMemoryPool::print_pool(bool short_format) const {
    std::stringstream ss;
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    for (const auto& txd : m_fee_index) {
      ss << "id: " << txd.id << std::endl;
      
      if (!short_format) {
        ss << storeToJson(txd.tx) << std::endl;
      }

      ss << "blobSize: " << txd.blobSize << std::endl
        << "fee: " << m_currency.formatAmount(txd.fee) << std::endl
        << "keptByBlock: " << (txd.keptByBlock ? 'T' : 'F') << std::endl
        << "max_used_block_height: " << txd.maxUsedBlock.height << std::endl
        << "max_used_block_id: " << txd.maxUsedBlock.id << std::endl
        << "last_failed_height: " << txd.lastFailedBlock.height << std::endl
        << "last_failed_id: " << txd.lastFailedBlock.id << std::endl
        << "received: " << std::ctime(&txd.receiveTime) << std::endl;
    }

    return ss.str();
  }
  //---------------------------------------------------------------------------------
  bool TxMemoryPool::fill_block_template(block_t& bl, size_t median_size, size_t maxCumulativeSize,
                                           uint64_t already_generated_coins, size_t& total_size, uint64_t& fee) {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);

    total_size = 0;
    fee = 0;

    size_t max_total_size = 2 * median_size - m_currency.minerTxBlobReservedSize();
    max_total_size = std::min(max_total_size, maxCumulativeSize);

    BlockTemplate blockTemplate;

    for (auto it = m_fee_index.rbegin(); it != m_fee_index.rend() && it->fee == 0; ++it) {
      const auto& txd = *it;

      if (m_currency.fusionTxMaxSize() < total_size + txd.blobSize) {
        continue;
      }

      transaction::transaction_check_info_t checkInfo(txd);
      if (is_transaction_ready_to_go(txd.tx, checkInfo) && blockTemplate.addTransaction(txd.id, txd.tx)) {
        total_size += txd.blobSize;
      }
    }

    for (auto i = m_fee_index.begin(); i != m_fee_index.end(); ++i) {
      const auto& txd = *i;

      size_t blockSizeLimit = (txd.fee == 0) ? median_size : max_total_size;
      if (blockSizeLimit < total_size + txd.blobSize) {
        continue;
      }

      transaction::transaction_check_info_t checkInfo(txd);
      bool ready = is_transaction_ready_to_go(txd.tx, checkInfo);

      // update item state
      m_fee_index.modify(i, [&checkInfo](transaction::transaction_check_info_t& item) {
        item = checkInfo;
      });

      if (ready && blockTemplate.addTransaction(txd.id, txd.tx)) {
        total_size += txd.blobSize;
        fee += txd.fee;
      }
    }

    bl.transactionHashes = blockTemplate.getTransactions();
    return true;
  }
  //---------------------------------------------------------------------------------
  bool TxMemoryPool::init() {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    if (!loadFromBinaryFile(*this, m_currency.txPoolFileName())) {
      logger(ERROR) << "Failed to load memory pool from file " << m_currency.txPoolFileName();

      m_transactions.clear();
      m_spent_key_images.clear();
      m_spentOutputs.clear();

      m_paymentIdIndex.clear();
      m_timestampIndex.clear();
    } else {
      buildIndices();
    }

    removeExpiredTransactions();

    // Ignore deserialization error
    return true;
  }
  //---------------------------------------------------------------------------------
  bool TxMemoryPool::deinit() {
    if (!storeToBinaryFile(*this, m_currency.txPoolFileName())) {
      logger(INFO) << "Failed to serialize memory pool to file " << m_currency.txPoolFileName();
    }

    m_paymentIdIndex.clear();
    m_timestampIndex.clear();
    
    return true;
  }

#define CURRENT_MEMPOOL_ARCHIVE_VER 1

  void serialize(transaction::transaction_details_t& td, ISerializer& s) {
    s(td.id, "id");
    s(td.blobSize, "blobSize");
    s(td.fee, "fee");
    s(td.tx, "tx");
    s(td.maxUsedBlock.height, "maxUsedBlock.height");
    s(td.maxUsedBlock.id, "maxUsedBlock.id");
    s(td.lastFailedBlock.height, "lastFailedBlock.height");
    s(td.lastFailedBlock.id, "lastFailedBlock.id");
    s(td.keptByBlock, "keptByBlock");
    s(reinterpret_cast<uint64_t&>(td.receiveTime), "receiveTime");
  }

  //---------------------------------------------------------------------------------
  void TxMemoryPool::serialize(ISerializer& s) {

    uint8_t version = CURRENT_MEMPOOL_ARCHIVE_VER;

    s(version, "version");

    std::cout << "Mempool version: " << (uint32_t) version << std::endl;

    if (version != CURRENT_MEMPOOL_ARCHIVE_VER) {
      return;
    }

    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);

    if (s.type() == ISerializer::INPUT) {
      m_transactions.clear();
      readSequence<transaction::transaction_details_t>(std::inserter(m_transactions, m_transactions.end()), "transactions", s);
    } else {
      writeSequence<transaction::transaction_details_t>(m_transactions.begin(), m_transactions.end(), "transactions", s);
    }

    KV_MEMBER(m_spent_key_images);
    KV_MEMBER(m_spentOutputs);
    KV_MEMBER(m_recentlyDeletedTransactions);
  }

  //---------------------------------------------------------------------------------
  void TxMemoryPool::on_idle() {
    m_txCheckInterval.call([this](){ return removeExpiredTransactions(); });
  }

  //---------------------------------------------------------------------------------
  bool TxMemoryPool::removeExpiredTransactions() {
    bool somethingRemoved = false;
    {
      std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);

      uint64_t now = m_timeProvider.now();

      for (auto it = m_recentlyDeletedTransactions.begin(); it != m_recentlyDeletedTransactions.end();) {
        uint64_t elapsedTimeSinceDeletion = now - it->second;
        if (elapsedTimeSinceDeletion > m_currency.numberOfPeriodsToForgetTxDeletedFromPool() * m_currency.mempoolTxLiveTime()) {
          it = m_recentlyDeletedTransactions.erase(it);
        } else {
          ++it;
        }
      }

      for (auto it = m_transactions.begin(); it != m_transactions.end();) {
        uint64_t txAge = now - it->receiveTime;
        bool remove = txAge > (it->keptByBlock ? m_currency.mempoolTxFromAltBlockLiveTime() : m_currency.mempoolTxLiveTime());

        if (remove) {
          logger(TRACE) << "Tx " << it->id << " removed from tx pool due to outdated, age: " << txAge;
          m_recentlyDeletedTransactions.emplace(it->id, now);
          it = removeTransaction(it);
          somethingRemoved = true;
        } else {
          ++it;
        }
      }
    }

    if (somethingRemoved) {
      m_observerManager.notify(&ITxPoolObserver::txDeletedFromPool);
    }

    return true;
  }

  TxMemoryPool::tx_container_t::iterator TxMemoryPool::removeTransaction(TxMemoryPool::tx_container_t::iterator i) {
    removeTransactionInputs(i->id, i->tx, i->keptByBlock);
    m_paymentIdIndex.remove(i->tx);
    m_timestampIndex.remove(i->receiveTime, i->id);
    return m_transactions.erase(i);
  }

  bool TxMemoryPool::removeTransactionInputs(const crypto::hash_t& tx_id, const transaction_t& tx, bool keptByBlock) {
    for (const auto& in : tx.inputs) {
      if (in.type() == typeid(key_input_t)) {
        const auto& txin = boost::get<key_input_t>(in);
        auto it = m_spent_key_images.find(txin.keyImage);
        if (!(it != m_spent_key_images.end())) { logger(ERROR, BRIGHT_RED) << "failed to find transaction input in key images. img=" << txin.keyImage << std::endl
          << "transaction id = " << tx_id; return false; }
        std::unordered_set<crypto::hash_t>& key_image_set = it->second;
        if (!(!key_image_set.empty())) { logger(ERROR, BRIGHT_RED) << "empty key_image set, img=" << txin.keyImage << std::endl
          << "transaction id = " << tx_id; return false; }

        auto it_in_set = key_image_set.find(tx_id);
        if (!(it_in_set != key_image_set.end())) { logger(ERROR, BRIGHT_RED) << "transaction id not found in key_image set, img=" << txin.keyImage << std::endl
          << "transaction id = " << tx_id; return false; }
        key_image_set.erase(it_in_set);
        if (key_image_set.empty()) {
          //it is now empty hash container for this key_image
          m_spent_key_images.erase(it);
        }
      } else if (in.type() == typeid(multi_signature_input_t)) {
        if (!keptByBlock) {
          const auto& msig = boost::get<multi_signature_input_t>(in);
          auto output = global_output_t(msig.amount, msig.outputIndex);
          assert(m_spentOutputs.count(output));
          m_spentOutputs.erase(output);
        }
      }
    }

    return true;
  }

  //---------------------------------------------------------------------------------
  bool TxMemoryPool::addTransactionInputs(const crypto::hash_t& id, const transaction_t& tx, bool keptByBlock) {
    // should not fail
    for (const auto& in : tx.inputs) {
      if (in.type() == typeid(key_input_t)) {
        const auto& txin = boost::get<key_input_t>(in);
        std::unordered_set<crypto::hash_t>& kei_image_set = m_spent_key_images[txin.keyImage];
        if (!(keptByBlock || kei_image_set.size() == 0)) {
          logger(ERROR, BRIGHT_RED)
              << "internal error: keptByBlock=" << keptByBlock
              << ",  kei_image_set.size()=" << kei_image_set.size() << ENDL
              << "txin.keyImage=" << txin.keyImage << ENDL << "tx_id=" << id;
          return false;
        }
        auto ins_res = kei_image_set.insert(id);
        if (!(ins_res.second)) {
          logger(ERROR, BRIGHT_RED) << "internal error: try to insert duplicate iterator in key_image set";
          return false;
        }
      } else if (in.type() == typeid(multi_signature_input_t)) {
        if (!keptByBlock) {
          const auto& msig = boost::get<multi_signature_input_t>(in);
          auto r = m_spentOutputs.insert(global_output_t(msig.amount, msig.outputIndex));
          (void)r;
          assert(r.second);
        }
      }
    }

    return true;
  }

  //---------------------------------------------------------------------------------
  bool TxMemoryPool::haveSpentInputs(const transaction_t& tx) const {
    for (const auto& in : tx.inputs) {
      if (in.type() == typeid(key_input_t)) {
        const auto& tokey_in = boost::get<key_input_t>(in);
        if (m_spent_key_images.count(tokey_in.keyImage)) {
          return true;
        }
      } else if (in.type() == typeid(multi_signature_input_t)) {
        const auto& msig = boost::get<multi_signature_input_t>(in);
        if (m_spentOutputs.count(global_output_t(msig.amount, msig.outputIndex))) {
          return true;
        }
      }
    }
    return false;
  }

  bool TxMemoryPool::addObserver(ITxPoolObserver* observer) {
    return m_observerManager.add(observer);
  }

  bool TxMemoryPool::removeObserver(ITxPoolObserver* observer) {
    return m_observerManager.remove(observer);
  }

  void TxMemoryPool::buildIndices() {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    for (auto it = m_transactions.begin(); it != m_transactions.end(); it++) {
      m_paymentIdIndex.add(it->tx);
      m_timestampIndex.add(it->receiveTime, it->id);
    }
  }

  bool TxMemoryPool::getTransactionIdsByPaymentId(const crypto::hash_t& paymentId, std::vector<crypto::hash_t>& transactionIds) {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    return m_paymentIdIndex.find(paymentId, transactionIds);
  }

  bool TxMemoryPool::getTransactionIdsByTimestamp(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t transactionsNumberLimit, std::vector<crypto::hash_t>& hashes, uint64_t& transactionsNumberWithinTimestamps) {
    std::lock_guard<std::recursive_mutex> lock(m_transactions_lock);
    return m_timestampIndex.find(timestampBegin, timestampEnd, transactionsNumberLimit, hashes, transactionsNumberWithinTimestamps);
  }
}
