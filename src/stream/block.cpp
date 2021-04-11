#include "stream/transaction.h"
#include "stream/block.h"

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

Reader &operator>>(Reader &i, core_state_info_t &v)
{
  i >> v.tx_pool_size;
  i >> v.blockchain_height;
  i >> v.mining_speed;
  i >> v.alternative_blocks;
  i.read((char *)v.top_block_id_str.data(), v.top_block_id_str.length());
  return i;
}

Writer &operator<<(Writer &o, const core_state_info_t &v)
{
  o << v.tx_pool_size;
  o << v.blockchain_height;
  o << v.mining_speed;
  o << v.alternative_blocks;
  o.write(v.top_block_id_str.data(), v.top_block_id_str.size());
  return o;
}
