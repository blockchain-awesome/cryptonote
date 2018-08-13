// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletSingle.h"

#include <string.h>
#include <time.h>

#include "WalletLegacy/WalletHelper.h"
#include "WalletLegacy/WalletLegacySerialization.h"
#include "WalletLegacy/WalletLegacySerializer.h"
#include "WalletLegacy/WalletUtils.h"

using namespace Crypto;

namespace
{

const uint64_t ACCOUN_CREATE_TIME_ACCURACY = 24 * 60 * 60;

class ContextCounterHolder
{
public:
  ContextCounterHolder(CryptoNote::WalletAsyncContextCounter &shutdowner) : m_shutdowner(shutdowner) {}
  ~ContextCounterHolder() { m_shutdowner.delAsyncContext(); }

private:
  CryptoNote::WalletAsyncContextCounter &m_shutdowner;
};

template <typename F>
void runAtomic(std::mutex &mutex, F f)
{
  std::unique_lock<std::mutex> lock(mutex);
  f();
}

class InitWaiter : public CryptoNote::IWalletLegacyObserver
{
public:
  InitWaiter() : future(promise.get_future()) {}

  virtual void initCompleted(std::error_code result) override
  {
    promise.set_value(result);
  }

  std::error_code waitInit()
  {
    return future.get();
  }

private:
  std::promise<std::error_code> promise;
  std::future<std::error_code> future;
};

// class SaveWaiter : public CryptoNote::IWalletLegacyObserver
// {
// public:
//   SaveWaiter() : future(promise.get_future()) {}

//   virtual void saveCompleted(std::error_code result) override
//   {
//     promise.set_value(result);
//   }

//   std::error_code waitSave()
//   {
//     return future.get();
//   }

// private:
//   std::promise<std::error_code> promise;
//   std::future<std::error_code> future;
// };

} //namespace

namespace CryptoNote
{

class SyncStarter : public CryptoNote::IWalletLegacyObserver
{
public:
  SyncStarter(BlockchainSynchronizer &sync) : m_sync(sync) {}
  virtual ~SyncStarter() {}

  virtual void initCompleted(std::error_code result) override
  {
    if (!result)
    {
      m_sync.start();
    }
  }

  BlockchainSynchronizer &m_sync;
};

WalletSingle::WalletSingle(const CryptoNote::Currency &currency, INode &node, Logging::LoggerGroup &logger) : m_state(NOT_INITIALIZED),
                                                                                                              m_currency(currency),
                                                                                                              m_node(node),
                                                                                                              m_isStopping(false),
                                                                                                              m_logger(logger),
                                                                                                              m_lastNotifiedActualBalance(0),
                                                                                                              m_lastNotifiedPendingBalance(0),
                                                                                                              m_blockchainSync(node, currency.genesisBlockHash()),
                                                                                                              m_transfersSync(currency, m_blockchainSync, node),
                                                                                                              m_transferDetails(nullptr),
                                                                                                              m_transactionsCache(m_currency.mempoolTxLiveTime()),
                                                                                                              m_sender(nullptr),
                                                                                                              m_onInitSyncStarter(new SyncStarter(m_blockchainSync))

{
  addObserver(m_onInitSyncStarter.get());
}

WalletSingle::~WalletSingle()
{
  removeObserver(m_onInitSyncStarter.get());

  {
    std::unique_lock<std::mutex> lock(m_cacheMutex);
    if (m_state != NOT_INITIALIZED)
    {
      m_sender->stop();
      m_isStopping = true;
    }
  }

  m_blockchainSync.removeObserver(this);
  m_blockchainSync.stop();
  m_asyncContextCounter.waitAsyncContextsFinish();
  m_sender.release();
}

void WalletSingle::addObserver(IWalletLegacyObserver *observer)
{
  m_observerManager.add(observer);
}

void WalletSingle::removeObserver(IWalletLegacyObserver *observer)
{
  m_observerManager.remove(observer);
}

void WalletSingle::initWithKeys(const AccountKeys &accountKeys, const std::string &password)
{
  {
    std::unique_lock<std::mutex> stateLock(m_cacheMutex);

    if (m_state != NOT_INITIALIZED)
    {
      throw std::system_error(make_error_code(error::ALREADY_INITIALIZED));
    }

    m_account.setAccountKeys(accountKeys);
    m_account.set_createtime(ACCOUN_CREATE_TIME_ACCURACY);
    m_password = password;
    Logging::LoggerRef(m_logger, "insidde sync ")(Logging::INFO, Logging::YELLOW) << "initwithkeys";
    initSync();
  }

  Logging::LoggerRef(m_logger, "initwithkeys")(Logging::INFO, Logging::YELLOW) << "initwithkeys";

  m_observerManager.notify(&IWalletLegacyObserver::initCompleted, std::error_code());
}

void WalletSingle::initAndGenerate(const std::string &password)
{
}

void WalletSingle::initAndLoad(std::istream &source, const std::string &password)
{
}

void WalletSingle::shutdown()
{
}

void WalletSingle::reset()
{
}

void WalletSingle::save(std::ostream &destination, bool saveDetailed, bool saveCache)
{
}

std::error_code WalletSingle::changePassword(const std::string &oldPassword, const std::string &newPassword)
{
  return std::error_code();
}

void WalletSingle::initCompleted(std::error_code code)
{

  Logging::LoggerRef(m_logger, "wallet initCompleted")(Logging::INFO, Logging::YELLOW) << "initCompleted error code: " << code;

  // if (!code)
  // {
  //   m_blockchainSync.start();
  // }

  // log()
}

void WalletSingle::initSync()
{
  AccountSubscription sub;
  sub.keys = reinterpret_cast<const AccountKeys &>(m_account.getAccountKeys());
  sub.transactionSpendableAge = 1;
  sub.syncStart.height = 0;
  sub.syncStart.timestamp = m_account.get_createtime() - ACCOUN_CREATE_TIME_ACCURACY;

  auto &subObject = m_transfersSync.addSubscription(sub);
  m_transferDetails = &subObject.getContainer();
  subObject.addObserver(this);

  m_sender.reset(new WalletTransactionSender(m_currency, m_transactionsCache, m_account.getAccountKeys(), *m_transferDetails));
  m_state = INITIALIZED;

  m_blockchainSync.addObserver(this);
}

std::string WalletSingle::getAddress()
{
  std::unique_lock<std::mutex> lock(m_cacheMutex);
  return m_currency.accountAddressAsString(m_account);
}

uint64_t WalletSingle::actualBalance()
{
  std::unique_lock<std::mutex> lock(m_cacheMutex);

  throwIfNotInitialised();
  return m_transferDetails->balance(ITransfersContainer::IncludeKeyUnlocked) -
         m_transactionsCache.unconfrimedOutsAmount();
}

uint64_t WalletSingle::pendingBalance()
{
  std::unique_lock<std::mutex> lock(m_cacheMutex);
  throwIfNotInitialised();

  uint64_t change = m_transactionsCache.unconfrimedOutsAmount() - m_transactionsCache.unconfirmedTransactionsAmount();
  return m_transferDetails->balance(ITransfersContainer::IncludeKeyNotUnlocked) + change;
}

size_t WalletSingle::getTransactionCount()
{
  std::unique_lock<std::mutex> lock(m_cacheMutex);
  throwIfNotInitialised();

  return m_transactionsCache.getTransactionCount();
}

size_t WalletSingle::getTransferCount()
{
  std::unique_lock<std::mutex> lock(m_cacheMutex);
  throwIfNotInitialised();

  return m_transactionsCache.getTransferCount();
}

TransactionId WalletSingle::findTransactionByTransferId(TransferId transferId)
{
  std::unique_lock<std::mutex> lock(m_cacheMutex);
  throwIfNotInitialised();

  return m_transactionsCache.findTransactionByTransferId(transferId);
}

bool WalletSingle::getTransaction(TransactionId transactionId, WalletLegacyTransaction &transaction)
{
  std::unique_lock<std::mutex> lock(m_cacheMutex);
  throwIfNotInitialised();

  return m_transactionsCache.getTransaction(transactionId, transaction);
}

bool WalletSingle::getTransfer(TransferId transferId, WalletLegacyTransfer &transfer)
{
  std::unique_lock<std::mutex> lock(m_cacheMutex);

  return m_transactionsCache.getTransfer(transferId, transfer);
}

TransactionId WalletSingle::sendTransaction(const WalletLegacyTransfer &transfer, uint64_t fee, const std::string &extra, uint64_t mixIn, uint64_t unlockTimestamp)
{
  std::vector<WalletLegacyTransfer> transfers;
  transfers.push_back(transfer);
  throwIfNotInitialised();

  return sendTransaction(transfers, fee, extra, mixIn, unlockTimestamp);
}

TransactionId WalletSingle::sendTransaction(const std::vector<WalletLegacyTransfer> &transfers, uint64_t fee, const std::string &extra, uint64_t mixIn, uint64_t unlockTimestamp)
{
  TransactionId txId = 0;
  std::shared_ptr<WalletRequest> request;
  std::deque<std::shared_ptr<WalletLegacyEvent>> events;
  throwIfNotInitialised();

  {
    std::unique_lock<std::mutex> lock(m_cacheMutex);
    request = m_sender->makeSendRequest(txId, events, transfers, fee, extra, mixIn, unlockTimestamp);
  }

  notifyClients(events);

  if (request)
  {
    m_asyncContextCounter.addAsyncContext();
    request->perform(m_node, std::bind(&WalletSingle::sendTransactionCallback, this, std::placeholders::_1, std::placeholders::_2));
  }

  return txId;
}

void WalletSingle::sendTransactionCallback(WalletRequest::Callback callback, std::error_code ec)
{
  ContextCounterHolder counterHolder(m_asyncContextCounter);
  std::deque<std::shared_ptr<WalletLegacyEvent>> events;

  boost::optional<std::shared_ptr<WalletRequest>> nextRequest;
  {
    std::unique_lock<std::mutex> lock(m_cacheMutex);
    callback(events, nextRequest, ec);
  }

  notifyClients(events);

  if (nextRequest)
  {
    m_asyncContextCounter.addAsyncContext();
    (*nextRequest)->perform(m_node, std::bind(&WalletSingle::synchronizationCallback, this, std::placeholders::_1, std::placeholders::_2));
  }
}

void WalletSingle::synchronizationCallback(WalletRequest::Callback callback, std::error_code ec)
{
  ContextCounterHolder counterHolder(m_asyncContextCounter);

  std::deque<std::shared_ptr<WalletLegacyEvent>> events;
  boost::optional<std::shared_ptr<WalletRequest>> nextRequest;
  {
    std::unique_lock<std::mutex> lock(m_cacheMutex);
    callback(events, nextRequest, ec);
  }

  notifyClients(events);

  if (nextRequest)
  {
    m_asyncContextCounter.addAsyncContext();
    (*nextRequest)->perform(m_node, std::bind(&WalletSingle::synchronizationCallback, this, std::placeholders::_1, std::placeholders::_2));
  }
}

std::error_code WalletSingle::cancelTransaction(size_t transactionId)
{
  return make_error_code(CryptoNote::error::TX_CANCEL_IMPOSSIBLE);
}

void WalletSingle::synchronizationProgressUpdated(uint32_t current, uint32_t total)
{
    Logging::LoggerRef(m_logger, "synchronizationProgressUpdated")(Logging::INFO, Logging::YELLOW) << "synchronizationProgressUpdated";
    Logging::LoggerRef(m_logger, "synchronizationProgressUpdated")(Logging::INFO, Logging::YELLOW) << "current" << current << ",total" << total;

  auto deletedTransactions = deleteOutdatedUnconfirmedTransactions();

  // forward notification
  m_observerManager.notify(&IWalletLegacyObserver::synchronizationProgressUpdated, current, total);

  for (auto transactionId : deletedTransactions)
  {
    m_observerManager.notify(&IWalletLegacyObserver::transactionUpdated, transactionId);
  }

  // check if balance has changed and notify client
  notifyIfBalanceChanged();
}

void WalletSingle::synchronizationCompleted(std::error_code result)
{

  Logging::LoggerRef(m_logger, "synchronizationCompleted")(Logging::INFO, Logging::YELLOW) << "synchronizationCompleted";

  if (result != std::make_error_code(std::errc::interrupted))
  {
    Logging::LoggerRef(m_logger, "not interrupted")(Logging::INFO, Logging::YELLOW) << "not interrupted";
    m_observerManager.notify(&IWalletLegacyObserver::synchronizationCompleted, result);
  }

  Logging::LoggerRef(m_logger, "result")(Logging::INFO, Logging::YELLOW) << result;

  if (result)
  {
    Logging::LoggerRef(m_logger, "return")(Logging::INFO, Logging::YELLOW) << "return";
    return;
  }

  auto deletedTransactions = deleteOutdatedUnconfirmedTransactions();
  std::for_each(deletedTransactions.begin(), deletedTransactions.end(), [&](TransactionId transactionId) {
    m_observerManager.notify(&IWalletLegacyObserver::transactionUpdated, transactionId);
  });

  notifyIfBalanceChanged();
}

void WalletSingle::onTransactionUpdated(ITransfersSubscription *object, const Hash &transactionHash)
{
  std::shared_ptr<WalletLegacyEvent> event;

  TransactionInformation txInfo;
  uint64_t amountIn;
  uint64_t amountOut;
  if (m_transferDetails->getTransactionInformation(transactionHash, txInfo, &amountIn, &amountOut))
  {
    std::unique_lock<std::mutex> lock(m_cacheMutex);
    event = m_transactionsCache.onTransactionUpdated(txInfo, static_cast<int64_t>(amountOut) - static_cast<int64_t>(amountIn));
  }

  if (event.get())
  {
    event->notify(m_observerManager);
  }
}

void WalletSingle::onTransactionDeleted(ITransfersSubscription *object, const Hash &transactionHash)
{
  std::shared_ptr<WalletLegacyEvent> event;

  {
    std::unique_lock<std::mutex> lock(m_cacheMutex);
    event = m_transactionsCache.onTransactionDeleted(transactionHash);
  }

  if (event.get())
  {
    event->notify(m_observerManager);
  }
}

void WalletSingle::throwIfNotInitialised()
{
  if (m_state == NOT_INITIALIZED || m_state == LOADING)
  {
    throw std::system_error(make_error_code(CryptoNote::error::NOT_INITIALIZED));
  }
  assert(m_transferDetails);
}

void WalletSingle::notifyClients(std::deque<std::shared_ptr<WalletLegacyEvent>> &events)
{
  while (!events.empty())
  {
    std::shared_ptr<WalletLegacyEvent> event = events.front();
    event->notify(m_observerManager);
    events.pop_front();
  }
}

void WalletSingle::notifyIfBalanceChanged()
{

  Logging::LoggerRef(m_logger, "notifyIfBalanceChanged")(Logging::INFO, Logging::YELLOW) << "notifyIfBalanceChanged";

  auto actual = actualBalance();
  auto prevActual = m_lastNotifiedActualBalance.exchange(actual);

  if (prevActual != actual)
  {
    m_observerManager.notify(&IWalletLegacyObserver::actualBalanceUpdated, actual);
  }

  auto pending = pendingBalance();
  auto prevPending = m_lastNotifiedPendingBalance.exchange(pending);

  if (prevPending != pending)
  {
    m_observerManager.notify(&IWalletLegacyObserver::pendingBalanceUpdated, pending);
  }
}

void WalletSingle::getAccountKeys(AccountKeys &keys)
{
  if (m_state == NOT_INITIALIZED)
  {
    throw std::system_error(make_error_code(CryptoNote::error::NOT_INITIALIZED));
  }

  keys = m_account.getAccountKeys();
}

std::vector<TransactionId> WalletSingle::deleteOutdatedUnconfirmedTransactions()
{
  std::lock_guard<std::mutex> lock(m_cacheMutex);
  return m_transactionsCache.deleteOutdatedTransactions();
}

} //namespace CryptoNote
