

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
    }
    catch (std::exception &)
    {
      return false;
    }

    return true;
  }

}