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

    Reader &operator>>(Reader &i, key_pair_t &v)
    {
      i >> v.secretKey;
      i >> v.publicKey;
      return i;
    }
    Writer &operator<<(Writer &o, const key_pair_t &v)
    {
      o << v.secretKey;
      o << v.publicKey;
      return o;
    }

    Reader &operator>>(Reader &i, account_public_address_t &v)
    {
      i >> v.spendPublicKey;
      i >> v.viewPublicKey;
      return i;
    }
    Writer &operator<<(Writer &o, const account_public_address_t &v)
    {
      o << v.spendPublicKey;
      o << v.viewPublicKey;
      return o;
    }

    Reader &operator>>(Reader &i, account_keys_t &v)
    {
      i >> v.address;
      i >> v.spendSecretKey;
      i >> v.viewSecretKey;
      return i;
    }

    Writer &operator<<(Writer &o, const account_keys_t &v)
    {
      o << v.address;
      o << v.spendSecretKey;
      o << v.viewSecretKey;
      return o;
    }
  }
}