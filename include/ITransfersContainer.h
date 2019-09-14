// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include "cryptonote/hash.h"
#include "ITransaction.h"
#include "IObservable.h"
#include "IStreamSerializable.h"

namespace cryptonote {

const uint32_t UNCONFIRMED_TRANSACTION_GLOBAL_OUTPUT_INDEX = std::numeric_limits<uint32_t>::max();

struct TransactionInformation {
  // transaction info
  hash_t transactionHash;
  public_key_t publicKey;
  uint32_t blockHeight;
  uint64_t timestamp;
  uint64_t unlockTime;
  uint64_t totalAmountIn;
  uint64_t totalAmountOut;
  std::vector<uint8_t> extra;
  hash_t paymentId;
};


struct TransactionOutputInformation {
  // output info
  TransactionTypes::output_type_t type;
  uint64_t amount;
  uint32_t globalOutputIndex;
  uint32_t outputInTransaction;

  // transaction info
  hash_t transactionHash;
  public_key_t transactionPublicKey;

  union {
    public_key_t outputKey;         // Type: Key 
    uint32_t requiredSignatures; // Type: Multisignature
  };
};

struct TransactionSpentOutputInformation: public TransactionOutputInformation {
  uint32_t spendingBlockHeight;
  uint64_t timestamp;
  hash_t spendingTransactionHash;
  key_image_t keyImage;  //!< \attention Used only for TransactionTypes::output_type_t::Key
  uint32_t inputInTransaction;
};

class ITransfersContainer : public IStreamSerializable {
public:
  enum Flags : uint32_t {
    // state
    IncludeStateUnlocked = 0x01,
    IncludeStateLocked = 0x02,
    IncludeStateSoftLocked = 0x04,
    IncludeStateSpent = 0x08,
    // output type
    IncludeTypeKey = 0x100,
    IncludeTypeMultisignature = 0x200,
    // combinations
    IncludeStateAll = 0xff,
    IncludeTypeAll = 0xff00,

    IncludeKeyUnlocked = IncludeTypeKey | IncludeStateUnlocked,
    IncludeKeyNotUnlocked = IncludeTypeKey | IncludeStateLocked | IncludeStateSoftLocked,

    IncludeAllLocked = IncludeTypeAll | IncludeStateLocked | IncludeStateSoftLocked,
    IncludeAllUnlocked = IncludeTypeAll | IncludeStateUnlocked,
    IncludeAll = IncludeTypeAll | IncludeStateAll,

    IncludeDefault = IncludeKeyUnlocked
  };

  virtual size_t transfersCount() const = 0;
  virtual size_t transactionsCount() const = 0;
  virtual uint64_t balance(uint32_t flags = IncludeDefault) const = 0;
  virtual void getOutputs(std::vector<TransactionOutputInformation>& transfers, uint32_t flags = IncludeDefault) const = 0;
  virtual bool getTransactionInformation(const hash_t& transactionHash, TransactionInformation& info,
    uint64_t* amountIn = nullptr, uint64_t* amountOut = nullptr) const = 0;
  virtual std::vector<TransactionOutputInformation> getTransactionOutputs(const hash_t& transactionHash, uint32_t flags = IncludeDefault) const = 0;
  //only type flags are feasible for this function
  virtual std::vector<TransactionOutputInformation> getTransactionInputs(const hash_t& transactionHash, uint32_t flags) const = 0;
  virtual void getUnconfirmedTransactions(std::vector<hash_t>& transactions) const = 0;
  virtual std::vector<TransactionSpentOutputInformation> getSpentOutputs() const = 0;
};

}
