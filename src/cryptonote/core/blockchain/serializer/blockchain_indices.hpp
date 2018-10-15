#pragma once

#include "crypto/hash.h"
#include <Logging/LoggerRef.h>
#include <Logging/ILogger.h>
#include <common/StdInputStream.h>
#include <Serialization/BinaryInputStreamSerializer.h>
#include <common/StdOutputStream.h>
#include <Serialization/BinaryOutputStreamSerializer.h>
#include <fstream>
#include <chrono>
#include <cryptonote/core/blockchain/defines.h>
#include <cryptonote/core/Blockchain.h>

using namespace Logging;
using namespace Common;

namespace cryptonote
{

class Blockchain;

class BlockchainIndicesSerializer
{

  public:
    BlockchainIndicesSerializer(Blockchain &bs, const crypto::Hash lastBlockHash, ILogger &logger) : m_bs(bs), m_lastBlockHash(lastBlockHash), m_loaded(false), logger(logger, "BlockchainIndicesSerializer")
    {
    }

    void serialize(ISerializer &s)
    {

        uint8_t version = CURRENT_BLOCKCHAININDICES_STORAGE_ARCHIVE_VER;

        KV_MEMBER(version);

        // ignore old versions, do rebuild
        if (version != CURRENT_BLOCKCHAININDICES_STORAGE_ARCHIVE_VER)
            return;

        std::string operation;

        if (s.type() == ISerializer::INPUT)
        {
            operation = "- loading ";

            crypto::Hash blockHash;
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

        // ignore old versions, do rebuild
        if (version < CURRENT_BLOCKCHAININDICES_STORAGE_ARCHIVE_VER)
            return;

        std::string operation;
        if (Archive::is_loading::value)
        {
            operation = "- loading ";
            crypto::Hash blockHash;
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
    crypto::Hash m_lastBlockHash;
};
} // namespace cryptonote