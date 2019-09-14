// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <future>
#include <map>
#include <mutex>

#include "cryptonote/hash.h"
#include "IWalletLegacy.h"

namespace cryptonote {
namespace WalletHelper {

class SaveWalletResultObserver : public cryptonote::IWalletLegacyObserver {
public:
  std::promise<std::error_code> saveResult;
  virtual void saveCompleted(std::error_code result) override { saveResult.set_value(result); }
};

class InitWalletResultObserver : public cryptonote::IWalletLegacyObserver {
public:
  std::promise<std::error_code> initResult;
  virtual void initCompleted(std::error_code result) override { initResult.set_value(result); }
};

class SendCompleteResultObserver : public cryptonote::IWalletLegacyObserver {
public:
  virtual void sendTransactionCompleted(cryptonote::TransactionId transactionId, std::error_code result) override;
  std::error_code wait(cryptonote::TransactionId transactionId);

private:
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::map<cryptonote::TransactionId, std::error_code> m_finishedTransactions;
  std::error_code m_result;
};

class IWalletRemoveObserverGuard {
public:
  IWalletRemoveObserverGuard(cryptonote::IWalletLegacy& wallet, cryptonote::IWalletLegacyObserver& observer);
  ~IWalletRemoveObserverGuard();

  void removeObserver();

private:
  cryptonote::IWalletLegacy& m_wallet;
  cryptonote::IWalletLegacyObserver& m_observer;
  bool m_removed;
};

void prepareFileNames(const std::string& file_path, std::string& keys_file, std::string& wallet_file);
void storeWallet(cryptonote::IWalletLegacy& wallet, const std::string& walletFilename);

}
}
