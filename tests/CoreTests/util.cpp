#include "util.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/transaction/serializer/basics.h"
#include "cryptonote/structures/block_entry.h"
#include "cryptonote/structures/array.hpp"


namespace cryptonote {

bool operator ==(const cryptonote::block_t& a, const cryptonote::block_t& b) {
  return cryptonote::Block::getHash(a) == cryptonote::Block::getHash(b);
}

bool operator ==(const cryptonote::transaction_t& a, const cryptonote::transaction_t& b) {
  return BinaryArray::objectHash(a) == BinaryArray::objectHash(b);
}

}
