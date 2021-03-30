#include "block_cache.h"
#include "stream/crypto.h"

using namespace serialize;

bool BlockCacheStream::serialize(std::ostream &o)
{
  o << version;
  o << m_lastBlockHash;

}
bool BlockCacheStream::serialize(std::istream &i)
{
}