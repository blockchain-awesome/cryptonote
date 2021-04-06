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

    void serialize(ISerializer &s)
    {

        config::config_t &data = config::get();
        uint8_t version = data.storageVersions.blockcache_archive.major;

        KV_MEMBER(version);

        // ignore old versions, do rebuild
        if (version != data.storageVersions.blockcache_indices_archive.major)
            return;

        std::string operation;

        if (s.type() == ISerializer::INPUT)
        {
            operation = "- loading ";

            hash_t blockHash;
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
        s(payment, "paymentIdIndex");

        logger(INFO) << operation << "timestamp index...";
        s(timestamp, "timestampIndex");

        logger(INFO) << operation << "generated transactions index...";
        s(transaction, "generatedTransactionsIndex");

        m_loaded = true;
    }

    // template <class Archive>
    // void serialize(Archive &ar, unsigned int version)
    // {
    //     uint8_t ver = config::get().storageVersions.blockcache_indices_archive.major;

    //     std::cout << "indexes version: " << ver 
    //     << " current version: " << version << std::endl;

    //     // ignore old versions, do rebuild
    //     if (version < ver)
    //         return;

    //     std::string operation;
    //     if (Archive::is_loading::value)
    //     {
    //         operation = "- loading ";
    //         hash_t blockHash;
    //         ar &blockHash;

    //         if (blockHash != m_lastBlockHash)
    //         {
    //             return;
    //         }
    //     }
    //     else
    //     {
    //         operation = "- saving ";
    //         ar &m_lastBlockHash;
    //     }

    //     logger(INFO) << operation << "paymentID index...";
    //     ar &m_bs.m_paymentIdIndex;

    //     logger(INFO) << operation << "timestamp index...";
    //     ar &m_bs.m_timestampIndex;

    //     logger(INFO) << operation << "generated transactions index...";
    //     ar &m_bs.m_generatedTransactionsIndex;

    //     m_loaded = true;
    // }

    bool loaded() const
    {
        return m_loaded;
    }

    LoggerRef logger;
    bool m_loaded;
    PaymentIdIndex& payment;
    TimestampBlocksIndex& timestamp;
    GeneratedTransactionsIndex& transaction;

    hash_t m_lastBlockHash;
};
} // namespace cryptonote