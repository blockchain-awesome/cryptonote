#include "stream/transaction.h"
#include "stream/block.h"

namespace stream
{
  namespace cryptonote
  {
    Reader &operator>>(Reader &i, block_header_t &v)
    {
      i >> v.majorVersion;
      i >> v.minorVersion;
      i >> v.previousBlockHash;
      i.read(&v.nonce, sizeof(v.nonce));
      return i;
    }
    Writer &operator<<(Writer &o, const block_header_t &v)
    {
      o << v.majorVersion;
      o << v.minorVersion;
      o << v.previousBlockHash;
      o.write(&v.nonce, sizeof(v.nonce));
      return o;
    }

    Reader &operator>>(Reader &i, block_t &v)
    {
      i >> static_cast<block_header_t &>(v);
      i >> v.baseTransaction;
      i >> v.transactionHashes;
      return i;
    }
    Writer &operator<<(Writer &o, const block_t &v)
    {
      o << static_cast<const block_header_t &>(v);
      o << v.baseTransaction;
      o << v.transactionHashes;
      return o;
    }
  }
}