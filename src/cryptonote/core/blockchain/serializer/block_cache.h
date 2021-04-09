#pragma once

#include "cryptonote/crypto/hash.h"
#include <logging/LoggerRef.h>
#include <logging/ILogger.h>
#include <serialization/BinaryInputStreamSerializer.h>
#include <stream/writer.h>
#include <serialization/BinaryOutputStreamSerializer.h>
#include <fstream>
#include <chrono>
#include <cryptonote/core/blockchain.h>
#include "stream/crypto.h"
#include "stream/cryptonote.h"
#include "stream/transaction.h"
#include "stream/map.hpp"
#include "stream/set.hpp"
using namespace Logging;
using namespace Common;

namespace cryptonote
{

  class Blockchain;
  class BlockCacheSerializer
  {

  public:
    BlockCacheSerializer(Blockchain &bs, const hash_t lastBlockHash, ILogger &logger) : m_bs(bs), m_lastBlockHash(lastBlockHash), m_loaded(false), logger(logger, "BlockCacheSerializer")
    {
    }

    bool loaded() const
    {
      return m_loaded;
    }

    void load(const std::string &filename);

    bool save(const std::string &filename);

    void serialize(ISerializer &s);

    LoggerRef logger;
    bool m_loaded;
    Blockchain &m_bs;
    hash_t m_lastBlockHash;
  };
  Reader &operator>>(Reader &i, BlockCacheSerializer &v);
  Writer &operator<<(Writer &o, const BlockCacheSerializer &v);
} // namespace cryptonote
