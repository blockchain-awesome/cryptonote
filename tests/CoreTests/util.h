#pragma once
#include "cryptonote/crypto/crypto.h"
#include "cryptonote/crypto/hash.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/blockchain/serializer/crypto.h"
namespace cryptonote {

bool operator ==(const cryptonote::block_t& a, const cryptonote::block_t& b);
bool operator ==(const cryptonote::transaction_t& a, const cryptonote::transaction_t& b);

}