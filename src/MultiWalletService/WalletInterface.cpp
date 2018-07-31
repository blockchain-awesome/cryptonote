// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <ctime>
#include <cassert>
#include <numeric>
#include <random>
#include <set>
#include <tuple>
#include <utility>

#include <iostream>

#include <System/EventLock.h>
#include <System/RemoteContext.h>


#include "Logging/LoggerGroup.h"

#include "ITransaction.h"

#include "Common/ScopeExit.h"
#include "Common/ShuffleGenerator.h"
#include "Common/StdInputStream.h"
#include "Common/StdOutputStream.h"
#include "Common/StringTools.h"
#include "CryptoNoteCore/Account.h"
#include "CryptoNoteCore/Currency.h"
#include "CryptoNoteCore/CryptoNoteFormatUtils.h"
#include "CryptoNoteCore/CryptoNoteTools.h"
#include "CryptoNoteCore/TransactionApi.h"
#include "crypto/crypto.h"
#include "Transfers/TransfersContainer.h"
#include "Wallet/WalletSerialization.h"
#include "Wallet/WalletErrors.h"
#include "Wallet/WalletUtils.h"

#include "WalletInterface.h"

using namespace Common;
using namespace Crypto;
using namespace CryptoNote;

using namespace std;

namespace
{

// void asyncRequestCompletion(System::Event &requestFinished)
// {
//   requestFinished.set();
// }

// void parseAddressString(const std::string &string, const CryptoNote::Currency &currency, CryptoNote::AccountPublicAddress &address)
// {
//   if (!currency.parseAccountAddressString(string, address))
//   {
//     throw std::system_error(make_error_code(CryptoNote::error::BAD_ADDRESS));
//   }
// }

// void validateAddresses(const std::vector<std::string> &addresses, const CryptoNote::Currency &currency)
// {
//   for (const auto &address : addresses)
//   {
//     if (!CryptoNote::validateAddress(address, currency))
//     {
//       throw std::system_error(make_error_code(CryptoNote::error::BAD_ADDRESS));
//     }
//   }
// }

// void validateOrders(const std::vector<WalletOrder> &orders, const CryptoNote::Currency &currency)
// {
//   for (const auto &order : orders)
//   {
//     if (!CryptoNote::validateAddress(order.address, currency))
//     {
//       throw std::system_error(make_error_code(CryptoNote::error::BAD_ADDRESS));
//     }

//     if (order.amount >= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
//     {
//       throw std::system_error(make_error_code(CryptoNote::error::WRONG_AMOUNT),
//                               "Order amount must not exceed " + std::to_string(std::numeric_limits<int64_t>::max()));
//     }
//   }
// }

// uint64_t countNeededMoney(const std::vector<CryptoNote::WalletTransfer> &destinations, uint64_t fee)
// {
//   uint64_t neededMoney = 0;
//   for (const auto &transfer : destinations)
//   {
//     if (transfer.amount == 0)
//     {
//       throw std::system_error(make_error_code(CryptoNote::error::ZERO_DESTINATION));
//     }
//     else if (transfer.amount < 0)
//     {
//       throw std::system_error(make_error_code(std::errc::invalid_argument));
//     }

//     //to supress warning
//     uint64_t uamount = static_cast<uint64_t>(transfer.amount);
//     neededMoney += uamount;
//     if (neededMoney < uamount)
//     {
//       throw std::system_error(make_error_code(CryptoNote::error::SUM_OVERFLOW));
//     }
//   }

//   neededMoney += fee;
//   if (neededMoney < fee)
//   {
//     throw std::system_error(make_error_code(CryptoNote::error::SUM_OVERFLOW));
//   }

//   return neededMoney;
// }

// void checkIfEnoughMixins(std::vector<CryptoNote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount> &mixinResult, uint64_t mixIn)
// {
//   auto notEnoughIt = std::find_if(mixinResult.begin(), mixinResult.end(),
//                                   [mixIn](const CryptoNote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount &ofa) { return ofa.outs.size() < mixIn; });

//   if (mixIn == 0 && mixinResult.empty())
//   {
//     throw std::system_error(make_error_code(CryptoNote::error::MIXIN_COUNT_TOO_BIG));
//   }

//   if (notEnoughIt != mixinResult.end())
//   {
//     throw std::system_error(make_error_code(CryptoNote::error::MIXIN_COUNT_TOO_BIG));
//   }
// }

// CryptoNote::WalletEvent makeTransactionUpdatedEvent(size_t id)
// {
//   CryptoNote::WalletEvent event;
//   event.type = CryptoNote::WalletEventType::TRANSACTION_UPDATED;
//   event.transactionUpdated.transactionIndex = id;

//   return event;
// }

// CryptoNote::WalletEvent makeTransactionCreatedEvent(size_t id)
// {
//   CryptoNote::WalletEvent event;
//   event.type = CryptoNote::WalletEventType::TRANSACTION_CREATED;
//   event.transactionCreated.transactionIndex = id;

//   return event;
// }

CryptoNote::WalletEvent makeSyncProgressUpdatedEvent(uint32_t current, uint32_t total)
{
  CryptoNote::WalletEvent event;
  event.type = CryptoNote::WalletEventType::SYNC_PROGRESS_UPDATED;
  event.synchronizationProgressUpdated.processedBlockCount = current;
  event.synchronizationProgressUpdated.totalBlockCount = total;

  return event;
}

CryptoNote::WalletEvent makeSyncCompletedEvent()
{
  CryptoNote::WalletEvent event;
  event.type = CryptoNote::WalletEventType::SYNC_COMPLETED;

  return event;
}

// std::vector<WalletTransfer> convertOrdersToTransfers(const std::vector<WalletOrder> &orders)
// {
//   std::vector<WalletTransfer> transfers;
//   transfers.reserve(orders.size());

//   for (const auto &order : orders)
//   {
//     WalletTransfer transfer;

//     if (order.amount > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
//     {
//       throw std::system_error(make_error_code(CryptoNote::error::WRONG_AMOUNT),
//                               "Order amount must not exceed " + std::to_string(std::numeric_limits<decltype(transfer.amount)>::max()));
//     }

//     transfer.type = WalletTransferType::USUAL;
//     transfer.address = order.address;
//     transfer.amount = static_cast<int64_t>(order.amount);

//     transfers.emplace_back(std::move(transfer));
//   }

//   return transfers;
// }

// uint64_t calculateDonationAmount(uint64_t freeAmount, uint64_t donationThreshold, uint64_t dustThreshold)
// {
//   std::vector<uint64_t> decomposedAmounts;
//   decomposeAmount(freeAmount, dustThreshold, decomposedAmounts);

//   std::sort(decomposedAmounts.begin(), decomposedAmounts.end(), std::greater<uint64_t>());

//   uint64_t donationAmount = 0;
//   for (auto amount : decomposedAmounts)
//   {
//     if (amount > donationThreshold - donationAmount)
//     {
//       continue;
//     }

//     donationAmount += amount;
//   }

//   assert(donationAmount <= freeAmount);
//   return donationAmount;
// }

// uint64_t pushDonationTransferIfPossible(const DonationSettings &donation, uint64_t freeAmount, uint64_t dustThreshold, std::vector<WalletTransfer> &destinations)
// {
//   uint64_t donationAmount = 0;
//   if (!donation.address.empty() && donation.threshold != 0)
//   {
//     if (donation.threshold > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
//     {
//       throw std::system_error(make_error_code(error::WRONG_AMOUNT),
//                               "Donation threshold must not exceed " + std::to_string(std::numeric_limits<int64_t>::max()));
//     }

//     donationAmount = calculateDonationAmount(freeAmount, donation.threshold, dustThreshold);
//     if (donationAmount != 0)
//     {
//       destinations.emplace_back(WalletTransfer{WalletTransferType::DONATION, donation.address, static_cast<int64_t>(donationAmount)});
//     }
//   }

//   return donationAmount;
// }

// CryptoNote::AccountPublicAddress parseAccountAddressString(const std::string &addressString, const CryptoNote::Currency &currency)
// {
//   CryptoNote::AccountPublicAddress address;

//   if (!currency.parseAccountAddressString(addressString, address))
//   {
//     throw std::system_error(make_error_code(CryptoNote::error::BAD_ADDRESS));
//   }

//   return address;
// }

} // namespace

namespace MultiWalletService
{

WalletInterface::WalletInterface(System::Dispatcher &dispatcher, const Currency &currency, INode &node, Logging::LoggerGroup &logger, uint32_t transactionSoftLockTime) : m_dispatcher(dispatcher),
                                                                                                                                            m_currency(currency),
                                                                                                                                            m_node(node),
                                                                                                                                            m_stopped(false),
                                                                                                                                            m_logger(logger),
                                                                                                                                            m_blockchainSynchronizerStarted(false),
                                                                                                                                            m_blockchainSynchronizer(node, currency.genesisBlockHash()),
                                                                                                                                            m_synchronizer(currency, m_blockchainSynchronizer, node),
                                                                                                                                            m_eventOccurred(m_dispatcher),
                                                                                                                                            m_readyEvent(m_dispatcher)
{
  m_upperTransactionSizeLimit = m_currency.blockGrantedFullRewardZone() * 2 - m_currency.minerTxBlobReservedSize();
  m_readyEvent.set();

  init();
}

WalletInterface::~WalletInterface()
{
  m_dispatcher.yield(); //let remote spawns finish
}

void WalletInterface::init()
{
  m_blockchain.push_back(m_currency.genesisBlockHash());
  m_blockchainSynchronizer.addObserver(this);
  Logging::LoggerRef log(m_logger, "multi wallet service");

  log(Logging::INFO) << "init wallet";
  startBlockchainSynchronizer();
}

void WalletInterface::synchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount)
{
  m_dispatcher.remoteSpawn([processedBlockCount, totalBlockCount, this]() { onSynchronizationProgressUpdated(processedBlockCount, totalBlockCount); });
}

void WalletInterface::synchronizationCompleted(std::error_code result)
{
  m_dispatcher.remoteSpawn([this]() { onSynchronizationCompleted(); });
}

void WalletInterface::onSynchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount)
{
  assert(processedBlockCount > 0);

  System::EventLock lk(m_readyEvent);

  // if (m_state == WalletState::NOT_INITIALIZED)
  // {
  //   return;
  // }

  pushEvent(makeSyncProgressUpdatedEvent(processedBlockCount, totalBlockCount));

  // uint32_t currentHeight = processedBlockCount - 1;

  Logging::LoggerRef(m_logger, "inteface")(Logging::INFO) << "on synchronizing" << endl;

  // unlockBalances(currentHeight);
}

void WalletInterface::onSynchronizationCompleted()
{
  System::EventLock lk(m_readyEvent);

  Logging::LoggerRef(m_logger, "inteface")(Logging::INFO) << "on synchronized" << endl;


  pushEvent(makeSyncCompletedEvent());
}

void WalletInterface::pushEvent(const WalletEvent &event)
{
  m_events.push(event);
  m_eventOccurred.set();
}

void WalletInterface::startBlockchainSynchronizer()
{
  // if (!m_walletsContainer.empty() && !m_blockchainSynchronizerStarted)
  if (!m_blockchainSynchronizerStarted)
  {
    m_blockchainSynchronizer.start();
    m_blockchainSynchronizerStarted = true;
  }
}

void WalletInterface::stopBlockchainSynchronizer()
{
  if (m_blockchainSynchronizerStarted)
  {
    m_blockchainSynchronizer.stop();
    m_blockchainSynchronizerStarted = false;
  }
}


Crypto::Hash WalletInterface::getBlockHashByIndex(uint32_t blockIndex) const
{
  assert(blockIndex < m_blockchain.size());
  return m_blockchain.get<BlockHeightIndex>()[blockIndex];
}

} // namespace MultiWalletService
