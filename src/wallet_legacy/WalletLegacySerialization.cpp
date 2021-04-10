// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletLegacySerialization.h"
#include "wallet_legacy/WalletUnconfirmedTransactions.h"
#include "IWalletLegacy.h"

#include "cryptonote/core/transaction/serializer/basics.h"
#include "serialization/ISerializer.h"
#include "serialization/SerializationOverloads.h"

namespace cryptonote
{

  void serialize(UnconfirmedTransferDetails &utd, cryptonote::ISerializer &serializer)
  {
    serializer(utd.tx, "transaction");
    serializer(utd.amount, "amount");
    serializer(utd.outsAmount, "outs_amount");
    uint64_t time = static_cast<uint64_t>(utd.sentTime);
    serializer(time, "sent_time");
    utd.sentTime = static_cast<time_t>(time);
    uint64_t txId = static_cast<uint64_t>(utd.transactionId);
    serializer(txId, "transaction_id");
    utd.transactionId = static_cast<size_t>(txId);
  }

  Reader &operator>>(Reader &i, UnconfirmedTransferDetails &v)
  {
    uint64_t time, id;
    i >> v.tx >> v.amount >> v.outsAmount >> time >> id;
    v.sentTime = static_cast<time_t>(time);
    v.transactionId = static_cast<size_t>(id);
    return i;
  }
  Writer &operator<<(Writer &o, const UnconfirmedTransferDetails &v)
  {
    o << v.tx << v.amount << v.outsAmount << static_cast<uint64_t>(v.sentTime)
      << static_cast<uint64_t>(v.transactionId);
    return o;
  }

  void serialize(WalletLegacyTransaction &txi, cryptonote::ISerializer &serializer)
  {
    uint64_t trId = static_cast<uint64_t>(txi.firstTransferId);
    serializer(trId, "first_transfer_id");
    txi.firstTransferId = static_cast<size_t>(trId);

    uint64_t trCount = static_cast<uint64_t>(txi.transferCount);
    serializer(trCount, "transfer_count");
    txi.transferCount = static_cast<size_t>(trCount);

    serializer(txi.totalAmount, "total_amount");

    serializer(txi.fee, "fee");
    serializer(txi.hash, "hash");
    serializer(txi.isCoinbase, "is_coinbase");

    cryptonote::serializeBlockHeight(serializer, txi.blockHeight, "block_height");

    serializer(txi.timestamp, "timestamp");
    serializer(txi.unlockTime, "unlock_time");
    serializer(txi.extra, "extra");

    //this field has been added later in the structure.
    //in order to not break backward binary compatibility
    // we just set it to zero
    txi.sentTime = 0;
  }

  Reader &operator>>(Reader &i, WalletLegacyTransaction &v)
  {
    uint64_t id, count, height;

    i >> id >> count >> v.totalAmount >> v.fee >> v.hash >> v.isCoinbase >> height >> v.timestamp >> v.extra;

    v.firstTransferId = static_cast<size_t>(id);
    v.transferCount = static_cast<size_t>(count);
    v.blockHeight = static_cast<uint32_t>(height);
    return i;
  }
  Writer &operator<<(Writer &o, const WalletLegacyTransaction &v)
  {
    o << static_cast<uint64_t>(v.firstTransferId)
      << static_cast<uint64_t>(v.transferCount)
      << v.totalAmount << v.fee << v.hash << v.isCoinbase << v.blockHeight
      << v.timestamp << v.extra;
    return o;
  }

  void serialize(WalletLegacyTransfer &tr, cryptonote::ISerializer &serializer)
  {
    serializer(tr.address, "address");
    serializer(tr.amount, "amount");
  }

  Reader &operator>>(Reader &i, WalletLegacyTransfer &v)
  {
    i >> v.address >> v.amount;
    return i;
  }
  Writer &operator<<(Writer &o, const WalletLegacyTransfer &v)
  {
    o << v.address << v.amount;
    return o;
  }

} //namespace cryptonote
