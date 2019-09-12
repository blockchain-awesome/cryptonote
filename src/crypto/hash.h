// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stddef.h>

#include <crypto.h>
#include "generic-ops.h"

namespace crypto
{

extern "C"
{
#include "hash-ops.h"
}

} // namespace crypto

CRYPTO_MAKE_HASHABLE(hash_t)
