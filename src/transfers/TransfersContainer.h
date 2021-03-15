// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <unordered_map>
#include <mutex>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "cryptonote/crypto/crypto.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/transaction/serializer/basics.h"
#include "cryptonote/core/currency.h"
#include "serialization/ISerializer.h"
#include "serialization/SerializationOverloads.h"

#include "ITransaction.h"
#include "ITransfersContainer.h"

namespace cryptonote {

struct TransactionOutputInformationIn;

class SpentOutputDescriptor {
public:
  SpentOutputDescriptor();
  SpentOutputDescriptor(const TransactionOutputInformationIn& transactionInfo);
  SpentOutputDescriptor(const key_image_t* keyImage);
  SpentOutputDescriptor(uint64_t amount, uint32_t globalOutputIndex);

  void assign(const key_image_t* keyImage);
  void assign(uint64_t amount, uint32_t globalOutputIndex);

  bool isValid() const;

  bool operator==(const SpentOutputDescriptor& other) const;
  size_t hash() const;

private:
  TransactionTypes::output_type_t m_type;
  union {
    const key_image_t* m_keyImage;
    struct {
      uint64_t m_amount;
      uint32_t m_globalOutputIndex;
    };
  };
};

struct SpentOutputDescriptorHasher {
  size_t operator()(const SpentOutputDescriptor& descriptor) const {
    return descriptor.hash();
  }
};

struct TransactionOutputInformationIn : public TransactionOutputInformation {
  key_image_t keyImage;  //!< \attention Used only for TransactionTypes::output_type_t::Key
};

struct TransactionOutputInformationEx : public TransactionOutputInformationIn {
  uint64_t unlockTime;
  uint32_t blockHeight;
  uint32_t transactionIndex;
  bool visible;

  SpentOutputDescriptor getSpentOutputDescriptor() const { return SpentOutputDescriptor(*this); }
  const hash_t& getTransactionHash() const { return transactionHash; }

  void serialize(cryptonote::ISerializer& s) {
    s(reinterpret_cast<uint8_t&>(type), "type");
    s(amount, "");
    serializeGlobalOutputIndex(s, globalOutputIndex, "");
    s(outputInTransaction, "");
    s(transactionPublicKey, "");
    s(keyImage, "");
    s(unlockTime, "");
    serializeBlockHeight(s, blockHeight, "");
    s(transactionIndex, "");
    s(transactionHash, "");
    s(visible, "");

    if (type == TransactionTypes::output_type_t::Key)
      s(outputKey, "");
    else if (type == TransactionTypes::output_type_t::Multisignature)
      s(requiredSignatures, "");
  }

};

struct TransactionBlockInfo {
  uint32_t height;
  uint64_t timestamp;
  uint32_t transactionIndex;

  void serialize(ISerializer& s) {
    serializeBlockHeight(s, height, "height");
    s(timestamp, "timestamp");
    s(transactionIndex, "transactionIndex");
  }
};

struct SpentTransactionOutput : TransactionOutputInformationEx {
  TransactionBlockInfo spendingBlock;
  hash_t spendingTransactionHash;
  uint32_t inputInTransaction;

  const hash_t& getSpendingTransactionHash() const {
    return spendingTransactionHash;
  }

  void serialize(ISerializer& s) {
    TransactionOutputInformationEx::serialize(s);
    s(spendingBlock, "spendingBlock");
    s(spendingTransactionHash, "spendingTransactionHash");
    s(inputInTransaction, "inputInTransaction");
  }
};

enum class KeyImageState {
  Unconfirmed,
  Confirmed,
  Spent
};

struct KeyOutputInfo {
  KeyImageState state;
  size_t count;
};

class TransfersContainer : public ITransfersContainer {

public:

  TransfersContainer(const cryptonote::Currency& currency, size_t transactionSpendableAge);

  bool addTransaction(const TransactionBlockInfo& block, const ITransactionReader& tx, const std::vector<TransactionOutputInformationIn>& transfers);
  bool deleteUnconfirmedTransaction(const hash_t& transactionHash);
  bool markTransactionConfirmed(const TransactionBlockInfo& block, const hash_t& transactionHash, const std::vector<uint32_t>& globalIndices);

  std::vector<hash_t> detach(uint32_t height);
  bool advanceHeight(uint32_t height);

  // ITransfersContainer
  virtual size_t transfersCount() const override;
  virtual size_t transactionsCount() const override;
  virtual uint64_t balance(uint32_t flags) const override;
  virtual void getOutputs(std::vector<TransactionOutputInformation>& transfers, uint32_t flags) const override;
  virtual bool getTransactionInformation(const hash_t& transactionHash, TransactionInformation& info,
    uint64_t* amountIn = nullptr, uint64_t* amountOut = nullptr) const override;
  virtual std::vector<TransactionOutputInformation> getTransactionOutputs(const hash_t& transactionHash, uint32_t flags) const override;
  //only type flags are feasible for this function
  virtual std::vector<TransactionOutputInformation> getTransactionInputs(const hash_t& transactionHash, uint32_t flags) const override;
  virtual void getUnconfirmedTransactions(std::vector<hash_t>& transactions) const override;
  virtual std::vector<TransactionSpentOutputInformation> getSpentOutputs() const override;

  // IStreamSerializable
  virtual void save(std::ostream& os) override;
  virtual void load(std::istream& in) override;

private:
  struct ContainingTransactionIndex { };
  struct SpendingTransactionIndex { };
  struct SpentOutputDescriptorIndex { };

  typedef boost::multi_index_container<
    TransactionInformation,
    boost::multi_index::indexed_by<
    boost::multi_index::hashed_unique<BOOST_MULTI_INDEX_MEMBER(TransactionInformation, hash_t, transactionHash)>,
    boost::multi_index::ordered_non_unique < BOOST_MULTI_INDEX_MEMBER(TransactionInformation, uint32_t, blockHeight) >
    >
  > TransactionMultiIndex;

  typedef boost::multi_index_container<
    TransactionOutputInformationEx,
    boost::multi_index::indexed_by<
      boost::multi_index::hashed_non_unique<
        boost::multi_index::tag<SpentOutputDescriptorIndex>,
        boost::multi_index::const_mem_fun<
          TransactionOutputInformationEx,
          SpentOutputDescriptor,
          &TransactionOutputInformationEx::getSpentOutputDescriptor>,
        SpentOutputDescriptorHasher
      >,
      boost::multi_index::hashed_non_unique<
        boost::multi_index::tag<ContainingTransactionIndex>,
        boost::multi_index::const_mem_fun<
          TransactionOutputInformationEx,
          const hash_t&,
          &TransactionOutputInformationEx::getTransactionHash>
      >
    >
  > UnconfirmedTransfersMultiIndex;

  typedef boost::multi_index_container<
    TransactionOutputInformationEx,
    boost::multi_index::indexed_by<
      boost::multi_index::hashed_non_unique<
        boost::multi_index::tag<SpentOutputDescriptorIndex>,
        boost::multi_index::const_mem_fun<
          TransactionOutputInformationEx,
          SpentOutputDescriptor,
          &TransactionOutputInformationEx::getSpentOutputDescriptor>,
        SpentOutputDescriptorHasher
      >,
      boost::multi_index::hashed_non_unique<
        boost::multi_index::tag<ContainingTransactionIndex>,
        boost::multi_index::const_mem_fun<
          TransactionOutputInformationEx,
          const hash_t&,
          &TransactionOutputInformationEx::getTransactionHash>
      >
    >
  > AvailableTransfersMultiIndex;

  typedef boost::multi_index_container<
    SpentTransactionOutput,
    boost::multi_index::indexed_by<
      boost::multi_index::hashed_unique<
        boost::multi_index::tag<SpentOutputDescriptorIndex>,
        boost::multi_index::const_mem_fun<
    TransactionOutputInformationEx,
          SpentOutputDescriptor,
          &TransactionOutputInformationEx::getSpentOutputDescriptor>,
        SpentOutputDescriptorHasher
      >,
      boost::multi_index::hashed_non_unique<
        boost::multi_index::tag<ContainingTransactionIndex>,
        boost::multi_index::const_mem_fun<
          TransactionOutputInformationEx,
          const hash_t&,
          &SpentTransactionOutput::getTransactionHash>
      >,
      boost::multi_index::hashed_non_unique <
        boost::multi_index::tag<SpendingTransactionIndex>,
        boost::multi_index::const_mem_fun <
          SpentTransactionOutput,
          const hash_t&,
          &SpentTransactionOutput::getSpendingTransactionHash>
      >
    >
  > SpentTransfersMultiIndex;

private:
  void addTransaction(const TransactionBlockInfo& block, const ITransactionReader& tx);
  bool addTransactionOutputs(const TransactionBlockInfo& block, const ITransactionReader& tx,
                             const std::vector<TransactionOutputInformationIn>& transfers);
  bool addTransactionInputs(const TransactionBlockInfo& block, const ITransactionReader& tx);
  void deleteTransactionTransfers(const hash_t& transactionHash);
  bool isSpendTimeUnlocked(uint64_t unlockTime) const;
  bool isIncluded(const TransactionOutputInformationEx& info, uint32_t flags) const;
  static bool isIncluded(TransactionTypes::output_type_t type, uint32_t state, uint32_t flags);
  void updateTransfersVisibility(const key_image_t& keyImage);

  void copyToSpent(const TransactionBlockInfo& block, const ITransactionReader& tx, size_t inputIndex, const TransactionOutputInformationEx& output);

private:
  TransactionMultiIndex m_transactions;
  UnconfirmedTransfersMultiIndex m_unconfirmedTransfers;
  AvailableTransfersMultiIndex m_availableTransfers;
  SpentTransfersMultiIndex m_spentTransfers;
  //std::unordered_map<key_image_t, KeyOutputInfo, boost::hash<key_image_t>> m_keyImages;

  uint32_t m_currentHeight; // current height is needed to check if a transfer is unlocked
  size_t m_transactionSpendableAge;
  const cryptonote::Currency& m_currency;
  mutable std::mutex m_mutex;
};

}
