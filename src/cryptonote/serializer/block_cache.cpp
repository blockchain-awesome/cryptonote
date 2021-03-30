#include "block_cache.h"
#include "stream/crypto.h"

using namespace serialize;
using namespace serialize::crypto;

BlockCacheStream::BlockCacheStream(BlockIndexStream &index) : index(index){};
bool BlockCacheStream::serialize(std::ostream &o)
{
  o << version;
  o << m_lastBlockHash;
  o << index;
}
bool BlockCacheStream::serialize(std::istream &i)
{
}