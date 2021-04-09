

#include "block_cache.h"

namespace cryptonote
{
    Reader &operator>>(Reader &i, BlockCacheSerializer &v)
    {
        config::config_t &data = config::get();

        uint8_t version = data.storageVersions.blockcache_archive.major;
        i >> version;
        if (version < data.storageVersions.blockcache_archive.major)
            return i;
        i >> v.m_lastBlockHash >> v.m_bs.m_blockIndex >> v.m_bs.m_transactionMap >> v.m_bs.m_outputs >> v.m_bs.m_multisignatureOutputs;
        return i;
    }

    Writer &operator<<(Writer &o, const BlockCacheSerializer &v)
    {
        auto start = std::chrono::steady_clock::now();
        config::config_t &data = config::get();

        uint8_t version = data.storageVersions.blockcache_archive.major;
        o << version
          << v.m_lastBlockHash
          << v.m_bs.m_blockIndex
          << v.m_bs.m_transactionMap
          << v.m_bs.m_outputs
          << v.m_bs.m_multisignatureOutputs;
        return o;
    }

    void BlockCacheSerializer::load(const std::string &filename)
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

            Reader stream(stdStream);

            stream >> *this;
            // BinaryInputStreamSerializer s(stream);
            // serialize(s);
        }
        catch (std::exception &e)
        {
            logger(WARNING) << "loading failed: " << e.what();
        }
    }

    bool BlockCacheSerializer::save(const std::string &filename)
    {
        try
        {
            std::ofstream file(filename, std::ios::binary);
            if (!file)
            {
                return false;
            }

            Writer stream(file);
            stream << *this;
            // BinaryOutputStreamSerializer s(stream);
            // serialize(s);
        }
        catch (std::exception &)
        {
            return false;
        }

        return true;
    }

    void BlockCacheSerializer::serialize(ISerializer &s)
    {
        auto start = std::chrono::steady_clock::now();

        config::config_t &data = config::get();
        uint8_t version = data.storageVersions.blockcache_archive.major;

        s(version, "version");

        std::cout << "Blockchain version: " << (int)version << std::endl;

        // ignore old versions, do rebuild
        if (version < data.storageVersions.blockcache_archive.major)
            return;

        std::string operation;
        if (s.type() == ISerializer::INPUT)
        {
            operation = "- loading ";
            hash_t blockHash;
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

}