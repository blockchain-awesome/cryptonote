#include "crypto/types.h"
#include "stream/serialize.h"

using namespace serialize;

class BlockCacheStream : public IStream
{
    bool serialize(std::ostream &o) const;
    bool serialize(std::istream &i) const;
    uint8_t version;
    hash_t m_lastBlockHash;
};