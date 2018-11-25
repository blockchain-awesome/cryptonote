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

class BlockchainIndicesSerializer
{

  public:
    BlockchainIndicesSerializer(Blockchain &bs, const crypto::hash_t lastBlockHash, ILogger &logger) : m_bs(bs), m_lastBlockHash(lastBlockHash), m_loaded(false), logger(logger, "BlockchainIndicesSerializer")
    {
    }

    void serialize(ISerializer &s)
    {

        config::config_t &data = config::get();
        uint8_t version = data.storage.blockcache_archive.major;

        KV_MEMBER(version);

        // ignore old versions, do rebuild
        if (version != data.storage.blockcache_indices_archive.major)
            return;

        std::string operation;

        if (s.type() == ISerializer::INPUT)
        {
            operation = "- loading ";

            crypto::hash_t blockHash;
            s(blockHash, "blockHash");

            if (blockHash != m_lastBlockHash)
            {
                return;
            }
        }
        else
        {
            operation = "- saving ";
            s(m_lastBlockHash, "blockHash");
        }

        logger(INFO) << operation << "paymentID index...";
        s(m_bs.m_paymentIdIndex, "paymentIdIndex");

        logger(INFO) << operation << "timestamp index...";
        s(m_bs.m_timestampIndex, "timestampIndex");

        logger(INFO) << operation << "generated transactions index...";
        s(m_bs.m_generatedTransactionsIndex, "generatedTransactionsIndex");

        m_loaded = true;
    }

    template <class Archive>
    void serialize(Archive &ar, unsigned int version)
    {
        uint8_t ver = config::get().storage.blockcache_indices_archive.major;

        std::cout << "indexes version: " << ver 
        << " current version: " << version << std::endl;

        // ignore old versions, do rebuild
        if (version < ver)
            return;

        std::string operation;
        if (Archive::is_loading::value)
        {
            operation = "- loading ";
            crypto::hash_t blockHash;
            ar &blockHash;

            if (blockHash != m_lastBlockHash)
            {
                return;
            }
        }
        else
        {
            operation = "- saving ";
            ar &m_lastBlockHash;
        }

        logger(INFO) << operation << "paymentID index...";
        ar &m_bs.m_paymentIdIndex;

        logger(INFO) << operation << "timestamp index...";
        ar &m_bs.m_timestampIndex;

        logger(INFO) << operation << "generated transactions index...";
        ar &m_bs.m_generatedTransactionsIndex;

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