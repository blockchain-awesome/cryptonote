// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <array>

#include "cryptonote/protocol/i_query.h"
#include "cryptonote/core/ICore.h"
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

  bool fillBlockDetails(const block_t& block, block_details_t& blockDetails);
  bool fillTransactionDetails(const transaction_t &tx, transaction_details_t& txRpcInfo, uint64_t timestamp = 0);

  static bool getPaymentId(const transaction_t& transaction, crypto::hash_t& paymentId);

private:
  bool getMixin(const transaction_t& transaction, uint64_t& mixin);
  bool fillTxExtra(const std::vector<uint8_t>& rawExtra, transaction_extra_details_t& extraDetails);
  size_t median(std::vector<size_t>& v);

  cryptonote::ICore& core;
  cryptonote::ICryptoNoteProtocolQuery& protocol;
};
}
