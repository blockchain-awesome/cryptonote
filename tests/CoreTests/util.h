#pragma once
#include "common/StringTools.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/serialize.h"
namespace cryptonote {

bool operator ==(const cryptonote::Block& a, const cryptonote::Block& b);
bool operator ==(const cryptonote::Transaction& a, const cryptonote::Transaction& b);

}