// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "IWallet.h"
#include "WalletIndices.h"
#include "stream/reader.h"
#include "stream/writer.h"
#include "transfers/TransfersSynchronizer.h"

#include "cryptonote/crypto/chacha.h"

namespace cryptonote {

struct CryptoContext {
  chacha_key_t key;
  chacha_iv_t iv;

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
  
  void save(const std::string& password, Writer& destination, bool saveDetails, bool saveCache);
  void load(const std::string& password, Reader& source);

private:
  static const uint32_t SERIALIZATION_VERSION;

  void loadWallet(Reader& source, const std::string& password, uint32_t version);
  void loadWalletV1(Reader& source, const std::string& password);

  CryptoContext generateCryptoContext(const std::string& password);

  void saveVersion(Writer& destination);
  void saveIv(Writer& destination, chacha_iv_t& iv);
  void saveKeys(Writer& destination, CryptoContext& cryptoContext);
  void savePublicKey(Writer& destination, CryptoContext& cryptoContext);
  void saveSecretKey(Writer& destination, CryptoContext& cryptoContext);
  void saveFlags(bool saveDetails, bool saveCache, Writer& destination, CryptoContext& cryptoContext);
  void saveWallets(Writer& destination, bool saveCache, CryptoContext& cryptoContext);
  void saveBalances(Writer& destination, bool saveCache, CryptoContext& cryptoContext);
  void saveTransfersSynchronizer(Writer& destination, CryptoContext& cryptoContext);
  void saveUnlockTransactionsJobs(Writer& destination, CryptoContext& cryptoContext);
  void saveUncommitedTransactions(Writer& destination, CryptoContext& cryptoContext);
  void saveTransactions(Writer& destination, CryptoContext& cryptoContext);
  void saveTransfers(Writer& destination, CryptoContext& cryptoContext);

  uint32_t loadVersion(Reader& source);
  void loadIv(Reader& source, chacha_iv_t& iv);
  void generateKey(const std::string& password, chacha_key_t& key);
  void loadKeys(Reader& source, CryptoContext& cryptoContext);
  void loadPublicKey(Reader& source, CryptoContext& cryptoContext);
  void loadSecretKey(Reader& source, CryptoContext& cryptoContext);
  void checkKeys();
  void loadFlags(bool& details, bool& cache, Reader& source, CryptoContext& cryptoContext);
  void loadWallets(Reader& source, CryptoContext& cryptoContext);
  void subscribeWallets();
  void loadBalances(Reader& source, CryptoContext& cryptoContext);
  void loadTransfersSynchronizer(Reader& source, CryptoContext& cryptoContext);
  void loadObsoleteSpentOutputs(Reader& source, CryptoContext& cryptoContext);
  void loadUnlockTransactionsJobs(Reader& source, CryptoContext& cryptoContext);
  void loadObsoleteChange(Reader& source, CryptoContext& cryptoContext);
  void loadUncommitedTransactions(Reader& source, CryptoContext& cryptoContext);
  void loadTransactions(Reader& source, CryptoContext& cryptoContext);
  void loadTransfers(Reader& source, CryptoContext& cryptoContext, uint32_t version);

  void loadWalletV1Keys(Reader &i);
  void loadWalletV1Details(Reader &i);
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
