#pragma once

#include "cryptonote/crypto/hash.h"
#include <logging/LoggerRef.h>
#include <logging/ILogger.h>
#include <stream/reader.h>
#include <serialization/BinaryInputStreamSerializer.h>
#include <stream/writer.h>
#include <serialization/BinaryOutputStreamSerializer.h>
#include <fstream>
#include <chrono>
#include <cryptonote/core/blockchain.h>

using namespace Logging;
using namespace Common;

namespace cryptonote
{

  class Blockchain;
  class PaymentIdIndex;
  class TimestampBlocksIndex;
  class GeneratedTransactionsIndex;

  class BlockchainIndicesSerializer
  {

  public:
    BlockchainIndicesSerializer(Blockchain &bs, const hash_t lastBlockHash, ILogger &logger) :

                                                                                               payment(bs.m_paymentIdIndex),
                                                                                               timestamp(bs.m_timestampIndex),
                                                                                               transaction(bs.m_generatedTransactionsIndex),
                                                                                               m_lastBlockHash(lastBlockHash), m_loaded(false), logger(logger, "BlockchainIndicesSerializer")
    {
    }

    bool loaded() const
    {
      return m_loaded;
    }

    LoggerRef logger;
    bool m_loaded;
    PaymentIdIndex &payment;
    TimestampBlocksIndex &timestamp;
    GeneratedTransactionsIndex &transaction;

    hash_t m_lastBlockHash;
  };
  Reader &operator>>(Reader &i, BlockchainIndicesSerializer &v)
  {
    config::config_t &data = config::get();
    uint8_t version = data.storageVersions.blockcache_archive.major;
    i >> version;

    // ignore old versions, do rebuild
    if (version != data.storageVersions.blockcache_indices_archive.major)
      return i;
    hash_t blockHash;
    i >> blockHash;

    if (blockHash != v.m_lastBlockHash)
    {
      return i;
    }

    i >> v.payment;
    i >> v.timestamp;
    i >> v.transaction;
    v.m_loaded = true;
    return i;
  }

  Writer &operator<<(Writer &o, const BlockchainIndicesSerializer &v)
  {
    config::config_t &data = config::get();
    uint8_t version = data.storageVersions.blockcache_archive.major;
    o << version;

    // ignore old versions, do rebuild
    if (version != data.storageVersions.blockcache_indices_archive.major)
      return o;

    o << v.m_lastBlockHash;
    o << v.payment;
    o << v.timestamp;
    o << v.transaction;
    return o;
  }

} // namespace cryptonote