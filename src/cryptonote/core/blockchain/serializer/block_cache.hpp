#pragma once

#include "crypto/hash.h"
#include <logging/LoggerRef.h>
#include <logging/ILogger.h>
#include <stream/StdInputStream.h>
#include <serialization/BinaryInputStreamSerializer.h>
#include <stream/StdOutputStream.h>
#include <serialization/BinaryOutputStreamSerializer.h>
#include <fstream>
#include <chrono>
#include <cryptonote/core/blockchain.h>

using namespace Logging;
using namespace Common;

namespace cryptonote
{

class Blockchain;

class BlockCacheSerializer
{

public:
  BlockCacheSerializer(Blockchain &bs, const crypto::hash_t lastBlockHash, ILogger &logger) : m_bs(bs), m_lastBlockHash(lastBlockHash), m_loaded(false), logger(logger, "BlockCacheSerializer")
  {
  }

  void load(const std::string &filename)
  {
    try
    {
      std::cout << "Loading cache from file: " << filename << std::endl;
      std::ifstream stdStream(filename, std::ios::binary);
      if (!stdStream)
      {
        std::cout << "Failed to open cached file: " << filename << std::endl;
        return;
      }

      StdInputStream stream(stdStream);
      BinaryInputStreamSerializer s(stream);
      serialize(s);
    }
    catch (std::exception &e)
    {
      logger(WARNING) << "loading failed: " << e.what();
    }
  }

  bool save(const std::string &filename)
  {
    try
    {
      std::ofstream file(filename, std::ios::binary);
      if (!file)
      {
        return false;
      }

      StdOutputStream stream(file);
      BinaryOutputStreamSerializer s(stream);
      serialize(s);
    }
    catch (std::exception &)
    {
      return false;
    }

    return true;
  }

  void serialize(ISerializer &s)
  {
    auto start = std::chrono::steady_clock::now();

    uint8_t version = config::mainnet::storage::blockcache_archive.major;

    s(version, "version");

    std::cout << "Blockchain version: " << (int)version << std::endl;

    // ignore old versions, do rebuild
    if (version < config::mainnet::storage::blockcache_archive.major)
      return;

    std::string operation;
    if (s.type() == ISerializer::INPUT)
    {
      operation = "- loading ";
      crypto::hash_t blockHash;
      s(blockHash, "last_block");

      if (blockHash != m_lastBlockHash)
      {
        return;
      }
    }
    else
    {
      operation = "- saving ";
      s(m_lastBlockHash, "last_block");
    }

    logger(INFO) << operation << "block index...";
    s(m_bs.m_blockIndex, "block_index");

    logger(INFO) << operation << "transaction map...";
    s(m_bs.m_transactionMap, "transactions");

    logger(INFO) << operation << "spent keys...";
    s(m_bs.m_spent_keys, "spent_keys");

    logger(INFO) << operation << "outputs...";
    s(m_bs.m_outputs, "outputs");

    logger(INFO) << operation << "multi-signature outputs...";
    s(m_bs.m_multisignatureOutputs, "multisig_outputs");

    auto dur = std::chrono::steady_clock::now() - start;

    logger(INFO) << "Serialization time: " << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "ms";

    m_loaded = true;
  }

  bool loaded() const
  {
    return m_loaded;
  }

private:
  LoggerRef logger;
  bool m_loaded;
  Blockchain &m_bs;
  crypto::hash_t m_lastBlockHash;
};

} // namespace cryptonote