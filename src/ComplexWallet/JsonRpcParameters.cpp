// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "JsonRpcParameters.h"
#include "Serialization/SerializationOverloads.h"

namespace ComplexWallet
{

void Login::Request::serialize(cryptonote::ISerializer &serializer)
{
  serializer(viewSecretKey, "viewSecretKey");
  serializer(sendSecretKey, "sendSecretKey");
  serializer(address, "address");
}

void Login::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(token, "token");
}

void GetViewKey::Request::serialize(cryptonote::ISerializer &serializer)
{
}

void GetViewKey::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(viewSecretKey, "viewSecretKey");
}

void GetStatus::Request::serialize(cryptonote::ISerializer &serializer)
{
}

void GetStatus::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(blockCount, "blockCount");
  serializer(knownBlockCount, "knownBlockCount");
  serializer(lastBlockHash, "lastBlockHash");
  serializer(peerCount, "peerCount");
}

void GetAddresses::Request::serialize(cryptonote::ISerializer &serializer)
{
}

void GetAddresses::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(addresses, "addresses");
}

void GetSpendKeys::Request::serialize(cryptonote::ISerializer &serializer)
{
  if (!serializer(address, "address"))
  {
    throw RequestSerializationError();
  }
}

void GetSpendKeys::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(spendSecretKey, "spendSecretKey");
  serializer(spendPublicKey, "spendPublicKey");
}

void GetBalance::Request::serialize(cryptonote::ISerializer &serializer)
{
  serializer(token, "token");
}

void GetBalance::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(availableBalance, "availableBalance");
  serializer(lockedAmount, "lockedAmount");
}

void GetBlockHashes::Request::serialize(cryptonote::ISerializer &serializer)
{
  bool r = serializer(firstBlockIndex, "firstBlockIndex");
  r &= serializer(blockCount, "blockCount");

  if (!r)
  {
    throw RequestSerializationError();
  }
}

void GetBlockHashes::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(blockHashes, "blockHashes");
}

void TransactionHashesInBlockRpcInfo::serialize(cryptonote::ISerializer &serializer)
{
  serializer(blockHash, "blockHash");
  serializer(transactionHashes, "transactionHashes");
}

void GetTransactionHashes::Request::serialize(cryptonote::ISerializer &serializer)
{
  serializer(addresses, "addresses");

  if (serializer(blockHash, "blockHash") == serializer(firstBlockIndex, "firstBlockIndex"))
  {
    throw RequestSerializationError();
  }

  if (!serializer(blockCount, "blockCount"))
  {
    throw RequestSerializationError();
  }

  serializer(paymentId, "paymentId");
}

void GetTransactionHashes::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(items, "items");
}

void TransferRpcInfo::serialize(cryptonote::ISerializer &serializer)
{
  serializer(type, "type");
  serializer(address, "address");
  serializer(amount, "amount");
}

void TransactionRpcInfo::serialize(cryptonote::ISerializer &serializer)
{
  serializer(state, "state");
  serializer(transactionHash, "transactionHash");
  serializer(blockIndex, "blockIndex");
  serializer(timestamp, "timestamp");
  serializer(isBase, "isBase");
  serializer(unlockTime, "unlockTime");
  serializer(amount, "amount");
  serializer(fee, "fee");
  serializer(transfers, "transfers");
  serializer(extra, "extra");
  serializer(paymentId, "paymentId");
}

void GetTransaction::Request::serialize(cryptonote::ISerializer &serializer)
{
  if (!serializer(transactionHash, "transactionHash"))
  {
    throw RequestSerializationError();
  }
}

void GetTransaction::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(transaction, "transaction");
}

void TransactionsInBlockRpcInfo::serialize(cryptonote::ISerializer &serializer)
{
  serializer(blockHash, "blockHash");
  serializer(transactions, "transactions");
}

void GetTransactions::Request::serialize(cryptonote::ISerializer &serializer)
{
  serializer(addresses, "addresses");

  if (serializer(blockHash, "blockHash") == serializer(firstBlockIndex, "firstBlockIndex"))
  {
    throw RequestSerializationError();
  }

  if (!serializer(blockCount, "blockCount"))
  {
    throw RequestSerializationError();
  }

  serializer(paymentId, "paymentId");
}

void GetTransactions::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(items, "items");
}

void GetUnconfirmedTransactionHashes::Request::serialize(cryptonote::ISerializer &serializer)
{
  serializer(addresses, "addresses");
}

void GetUnconfirmedTransactionHashes::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(transactionHashes, "transactionHashes");
}

void WalletRpcOrder::serialize(cryptonote::ISerializer &serializer)
{
  bool r = serializer(address, "address");
  r &= serializer(amount, "amount");

  if (!r)
  {
    throw RequestSerializationError();
  }
}

void SendTransaction::Request::serialize(cryptonote::ISerializer &serializer)
{
  serializer(sourceAddresses, "addresses");

  if (!serializer(transfers, "transfers"))
  {
    throw RequestSerializationError();
  }

  serializer(changeAddress, "changeAddress");

  if (!serializer(fee, "fee"))
  {
    throw RequestSerializationError();
  }

  if (!serializer(anonymity, "anonymity"))
  {
    throw RequestSerializationError();
  }

  bool hasExtra = serializer(extra, "extra");
  bool hasPaymentId = serializer(paymentId, "paymentId");

  if (hasExtra && hasPaymentId)
  {
    throw RequestSerializationError();
  }

  serializer(unlockTime, "unlockTime");
}

void SendTransaction::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(transactionHash, "transactionHash");
}

void CreateDelayedTransaction::Request::serialize(cryptonote::ISerializer &serializer)
{
  serializer(addresses, "addresses");

  if (!serializer(transfers, "transfers"))
  {
    throw RequestSerializationError();
  }

  serializer(changeAddress, "changeAddress");

  if (!serializer(fee, "fee"))
  {
    throw RequestSerializationError();
  }

  if (!serializer(anonymity, "anonymity"))
  {
    throw RequestSerializationError();
  }

  bool hasExtra = serializer(extra, "extra");
  bool hasPaymentId = serializer(paymentId, "paymentId");

  if (hasExtra && hasPaymentId)
  {
    throw RequestSerializationError();
  }

  serializer(unlockTime, "unlockTime");
}

void CreateDelayedTransaction::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(transactionHash, "transactionHash");
}

void GetDelayedTransactionHashes::Request::serialize(cryptonote::ISerializer &serializer)
{
}

void GetDelayedTransactionHashes::Response::serialize(cryptonote::ISerializer &serializer)
{
  serializer(transactionHashes, "transactionHashes");
}

void DeleteDelayedTransaction::Request::serialize(cryptonote::ISerializer &serializer)
{
  if (!serializer(transactionHash, "transactionHash"))
  {
    throw RequestSerializationError();
  }
}

void DeleteDelayedTransaction::Response::serialize(cryptonote::ISerializer &serializer)
{
}

void SendDelayedTransaction::Request::serialize(cryptonote::ISerializer &serializer)
{
  if (!serializer(transactionHash, "transactionHash"))
  {
    throw RequestSerializationError();
  }
}

void SendDelayedTransaction::Response::serialize(cryptonote::ISerializer &serializer)
{
}

} // namespace ComplexWallet
