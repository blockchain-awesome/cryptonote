// #include "block_cache.h"
// #include "stream/crypto.h"

// using namespace stream;
// using namespace stream::crypto;

// BlockCacheStream::BlockCacheStream(BlockIndexStream &index) : index(index){};
// bool BlockCacheStream::serialize(std::ostream &o)
// {
//   o << version;
//   o << m_lastBlockHash;
//   o << index;
//   o << map;
//   o << m_spent_keys;
// }

// bool BlockCacheStream::serialize(std::istream &i)
// {
//   i >> version;
//   i >> m_lastBlockHash;
//   i >> index;
//   i >> map;
//   i >> m_spent_keys;

// }