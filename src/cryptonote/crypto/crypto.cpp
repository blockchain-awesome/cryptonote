// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <alloca.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>

#include "crypto.h"
#include "hash.h"

namespace crypto
{

using std::abort;
using std::int32_t;
using std::lock_guard;
using std::mutex;

extern "C"
{
#include "crypto/crypto-ops.h"
#include "crypto/random.h"
#include "crypto/crypto-defines.h"
}

mutex random_lock;

#ifdef _MSC_VER
#pragma warning(disable : 4200)
#endif

} // namespace crypto
