// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <array>

#include "cryptonote_protocol/ICryptoNoteProtocolQuery.h"
#include "cryptonote_core/ICore.h"
#include "BlockchainExplorerData.h"

namespace cryptonote {

class BlockchainExplorerDataBuilder
{
public:
  BlockchainExplorerDataBuilder(cryptonote::ICore& core, cryptonote::ICryptoNoteProtocolQuery& protocol);

  BlockchainExplorerDataBuilder(const BlockchainExplorerDataBuilder&) = delete;
  BlockchainExplorerDataBuilder(BlockchainExplorerDataBuilder&&) = delete;

  BlockchainExplorerDataBuilder& operator=(const BlockchainExplorerDataBuilder&) = delete;
  BlockchainExplorerDataBuilder& operator=(BlockchainExplorerDataBuilder&&) = delete;

  bool fillBlockDetails(const Block& block, BlockDetails& blockDetails);
  bool fillTransactionDetails(const Transaction &tx, TransactionDetails& txRpcInfo, uint64_t timestamp = 0);

  static bool getPaymentId(const Transaction& transaction, crypto::Hash& paymentId);

private:
  bool getMixin(const Transaction& transaction, uint64_t& mixin);
  bool fillTxExtra(const std::vector<uint8_t>& rawExtra, TransactionExtraDetails& extraDetails);
  size_t median(std::vector<size_t>& v);

  cryptonote::ICore& core;
  cryptonote::ICryptoNoteProtocolQuery& protocol;
};
}
