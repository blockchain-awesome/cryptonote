// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "cryptonote/core/account.h"
#include "cryptonote/core/currency.h"

#include "INode.h"
#include "wallet_legacy/WalletSendTransactionContext.h"
#include "wallet_legacy/WalletUserTransactionsCache.h"
#include "wallet_legacy/WalletUnconfirmedTransactions.h"
#include "wallet_legacy/WalletRequest.h"

#include "ITransfersContainer.h"

namespace cryptonote {

class WalletTransactionSender
{
public:
  WalletTransactionSender(const Currency& currency, WalletUserTransactionsCache& transactionsCache, account_keys_t keys, ITransfersContainer& transfersContainer);

  void stop();

  std::shared_ptr<WalletRequest> makeSendRequest(TransactionId& transactionId, std::deque<std::shared_ptr<WalletLegacyEvent>>& events,
    const std::vector<WalletLegacyTransfer>& transfers, uint64_t fee, const std::string& extra = "", uint64_t mixIn = 0, uint64_t unlockTimestamp = 0);

private:
  std::shared_ptr<WalletRequest> makeGetRandomOutsRequest(std::shared_ptr<SendTransactionContext> context);
  std::shared_ptr<WalletRequest> doSendTransaction(std::shared_ptr<SendTransactionContext> context, std::deque<std::shared_ptr<WalletLegacyEvent>>& events);
  void prepareInputs(const std::list<TransactionOutputInformation>& selectedTransfers, std::vector<COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount>& outs,
      std::vector<transaction_source_entry_t>& sources, uint64_t mixIn);
  void splitDestinations(TransferId firstTransferId, size_t transfersCount, const transaction_destination_entry_t& changeDts,
    const TxDustPolicy& dustPolicy, std::vector<transaction_destination_entry_t>& splittedDests);
  void digitSplitStrategy(TransferId firstTransferId, size_t transfersCount, const transaction_destination_entry_t& change_dst, uint64_t dust_threshold,
    std::vector<transaction_destination_entry_t>& splitted_dsts, uint64_t& dust);
  void sendTransactionRandomOutsByAmount(std::shared_ptr<SendTransactionContext> context, std::deque<std::shared_ptr<WalletLegacyEvent>>& events,
      boost::optional<std::shared_ptr<WalletRequest> >& nextRequest, std::error_code ec);
  void relayTransactionCallback(std::shared_ptr<SendTransactionContext> context, std::deque<std::shared_ptr<WalletLegacyEvent>>& events,
                                boost::optional<std::shared_ptr<WalletRequest> >& nextRequest, std::error_code ec);
  void notifyBalanceChanged(std::deque<std::shared_ptr<WalletLegacyEvent>>& events);

  void validateTransfersAddresses(const std::vector<WalletLegacyTransfer>& transfers);
  bool validateDestinationAddress(const std::string& address);

  uint64_t selectTransfersToSend(uint64_t neededMoney, bool addDust, uint64_t dust, std::list<TransactionOutputInformation>& selectedTransfers);

  const Currency& m_currency;
  account_keys_t m_keys;
  WalletUserTransactionsCache& m_transactionsCache;
  uint64_t m_upperTransactionSizeLimit;

  bool m_isStoping;
  ITransfersContainer& m_transferDetails;
};

} /* namespace cryptonote */
