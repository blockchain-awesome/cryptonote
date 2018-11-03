// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <map>
#include <unordered_map>

#include "ITransfersContainer.h"
#include "IWallet.h"
#include "IWalletLegacy.h" //TODO: make common types for all of our APIs (such as public_key_t, KeyPair, etc)

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>

namespace cryptonote {

const uint64_t ACCOUNT_CREATE_TIME_ACCURACY = 60 * 60 * 24;

struct WalletRecord {
  crypto::public_key_t spendPublicKey;
  crypto::secret_key_t spendSecretKey;
  cryptonote::ITransfersContainer* container = nullptr;
  uint64_t pendingBalance = 0;
  uint64_t actualBalance = 0;
  time_t creationTimestamp;
};

struct RandomAccessIndex {};
struct KeysIndex {};
struct TransfersContainerIndex {};

struct WalletIndex {};
struct TransactionOutputIndex {};
struct BlockHeightIndex {};

struct TransactionHashIndex {};
struct TransactionIndex {};
struct BlockHashIndex {};

typedef boost::multi_index_container <
  WalletRecord,
  boost::multi_index::indexed_by <
    boost::multi_index::random_access < boost::multi_index::tag <RandomAccessIndex> >,
    boost::multi_index::hashed_unique < boost::multi_index::tag <KeysIndex>,
    BOOST_MULTI_INDEX_MEMBER(WalletRecord, crypto::public_key_t, spendPublicKey)>,
    boost::multi_index::hashed_unique < boost::multi_index::tag <TransfersContainerIndex>,
      BOOST_MULTI_INDEX_MEMBER(WalletRecord, cryptonote::ITransfersContainer*, container) >
  >
> WalletsContainer;

struct UnlockTransactionJob {
  uint32_t blockHeight;
  cryptonote::ITransfersContainer* container;
  crypto::hash_t transactionHash;
};

typedef boost::multi_index_container <
  UnlockTransactionJob,
  boost::multi_index::indexed_by <
    boost::multi_index::ordered_non_unique < boost::multi_index::tag <BlockHeightIndex>,
    BOOST_MULTI_INDEX_MEMBER(UnlockTransactionJob, uint32_t, blockHeight)
    >,
    boost::multi_index::hashed_non_unique < boost::multi_index::tag <TransactionHashIndex>,
      BOOST_MULTI_INDEX_MEMBER(UnlockTransactionJob, crypto::hash_t, transactionHash)
    >
  >
> UnlockTransactionJobs;

typedef boost::multi_index_container <
  cryptonote::WalletTransaction,
  boost::multi_index::indexed_by <
    boost::multi_index::random_access < boost::multi_index::tag <RandomAccessIndex> >,
    boost::multi_index::hashed_unique < boost::multi_index::tag <TransactionIndex>,
      boost::multi_index::member<cryptonote::WalletTransaction, crypto::hash_t, &cryptonote::WalletTransaction::hash >
    >,
    boost::multi_index::ordered_non_unique < boost::multi_index::tag <BlockHeightIndex>,
      boost::multi_index::member<cryptonote::WalletTransaction, uint32_t, &cryptonote::WalletTransaction::blockHeight >
    >
  >
> WalletTransactions;

typedef std::pair<size_t, cryptonote::WalletTransfer> TransactionTransferPair;
typedef std::vector<TransactionTransferPair> WalletTransfers;
typedef std::map<size_t, cryptonote::Transaction> UncommitedTransactions;

typedef boost::multi_index_container<
  crypto::hash_t,
  boost::multi_index::indexed_by <
    boost::multi_index::random_access<
      boost::multi_index::tag<BlockHeightIndex>
    >,
    boost::multi_index::hashed_unique<
      boost::multi_index::tag<BlockHashIndex>,
      boost::multi_index::identity<crypto::hash_t>
    >
  >
> BlockHashesContainer;

}
