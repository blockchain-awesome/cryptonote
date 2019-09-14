// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "IWallet.h"
#include "WalletIndices.h"
#include "stream/IInputStream.h"
#include "stream/IOutputStream.h"
#include "transfers/TransfersSynchronizer.h"
#include "serialization/BinaryInputStreamSerializer.h"

#include "cryptonote/crypto/chacha.h"

namespace cryptonote {

struct CryptoContext {
  crypto::chacha_key_t key;
  crypto::chacha_iv_t iv;

  void incIv();
};

class WalletSerializer {
public:
  WalletSerializer(
    ITransfersObserver& transfersObserver,
    public_key_t& viewPublicKey,
    secret_key_t& viewSecretKey,
    uint64_t& actualBalance,
    uint64_t& pendingBalance,
    WalletsContainer& walletsContainer,
    TransfersSyncronizer& synchronizer,
    UnlockTransactionJobs& unlockTransactions,
    WalletTransactions& transactions,
    WalletTransfers& transfers,
    uint32_t transactionSoftLockTime,
    UncommitedTransactions& uncommitedTransactions
  );
  
  void save(const std::string& password, Common::IOutputStream& destination, bool saveDetails, bool saveCache);
  void load(const std::string& password, Common::IInputStream& source);

private:
  static const uint32_t SERIALIZATION_VERSION;

  void loadWallet(Common::IInputStream& source, const std::string& password, uint32_t version);
  void loadWalletV1(Common::IInputStream& source, const std::string& password);

  CryptoContext generateCryptoContext(const std::string& password);

  void saveVersion(Common::IOutputStream& destination);
  void saveIv(Common::IOutputStream& destination, crypto::chacha_iv_t& iv);
  void saveKeys(Common::IOutputStream& destination, CryptoContext& cryptoContext);
  void savePublicKey(Common::IOutputStream& destination, CryptoContext& cryptoContext);
  void saveSecretKey(Common::IOutputStream& destination, CryptoContext& cryptoContext);
  void saveFlags(bool saveDetails, bool saveCache, Common::IOutputStream& destination, CryptoContext& cryptoContext);
  void saveWallets(Common::IOutputStream& destination, bool saveCache, CryptoContext& cryptoContext);
  void saveBalances(Common::IOutputStream& destination, bool saveCache, CryptoContext& cryptoContext);
  void saveTransfersSynchronizer(Common::IOutputStream& destination, CryptoContext& cryptoContext);
  void saveUnlockTransactionsJobs(Common::IOutputStream& destination, CryptoContext& cryptoContext);
  void saveUncommitedTransactions(Common::IOutputStream& destination, CryptoContext& cryptoContext);
  void saveTransactions(Common::IOutputStream& destination, CryptoContext& cryptoContext);
  void saveTransfers(Common::IOutputStream& destination, CryptoContext& cryptoContext);

  uint32_t loadVersion(Common::IInputStream& source);
  void loadIv(Common::IInputStream& source, crypto::chacha_iv_t& iv);
  void generateKey(const std::string& password, crypto::chacha_key_t& key);
  void loadKeys(Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadPublicKey(Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadSecretKey(Common::IInputStream& source, CryptoContext& cryptoContext);
  void checkKeys();
  void loadFlags(bool& details, bool& cache, Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadWallets(Common::IInputStream& source, CryptoContext& cryptoContext);
  void subscribeWallets();
  void loadBalances(Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadTransfersSynchronizer(Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadObsoleteSpentOutputs(Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadUnlockTransactionsJobs(Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadObsoleteChange(Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadUncommitedTransactions(Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadTransactions(Common::IInputStream& source, CryptoContext& cryptoContext);
  void loadTransfers(Common::IInputStream& source, CryptoContext& cryptoContext, uint32_t version);

  void loadWalletV1Keys(cryptonote::BinaryInputStreamSerializer& serializer);
  void loadWalletV1Details(cryptonote::BinaryInputStreamSerializer& serializer);
  void addWalletV1Details(const std::vector<WalletLegacyTransaction>& txs, const std::vector<WalletLegacyTransfer>& trs);
  void initTransactionPool();
  void resetCachedBalance();
  void updateTransactionsBaseStatus();
  void updateTransfersSign();

  ITransfersObserver& m_transfersObserver;
  public_key_t& m_viewPublicKey;
  secret_key_t& m_viewSecretKey;
  uint64_t& m_actualBalance;
  uint64_t& m_pendingBalance;
  WalletsContainer& m_walletsContainer;
  TransfersSyncronizer& m_synchronizer;
  UnlockTransactionJobs& m_unlockTransactions;
  WalletTransactions& m_transactions;
  WalletTransfers& m_transfers;
  uint32_t m_transactionSoftLockTime;
  UncommitedTransactions& uncommitedTransactions;
};

} //namespace cryptonote
