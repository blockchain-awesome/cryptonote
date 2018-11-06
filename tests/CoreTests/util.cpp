#include "util.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/blockchain/serializer/basics.h"


namespace cryptonote {

bool operator ==(const cryptonote::block_t& a, const cryptonote::block_t& b) {
  return cryptonote::get_block_hash(a) == cryptonote::get_block_hash(b);
}

bool operator ==(const cryptonote::transaction_t& a, const cryptonote::transaction_t& b) {
  return getObjectHash(a) == getObjectHash(b);
}

}