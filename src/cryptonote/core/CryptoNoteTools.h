// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <limits>
#include "stream/reader.h"
#include "common/StringTools.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "cryptonote/core/transaction/serializer/basics.h"

namespace cryptonote {

uint64_t getInputAmount(const transaction_t& transaction);
std::vector<uint64_t> getInputsAmounts(const transaction_t& transaction);
uint64_t getOutputAmount(const transaction_t& transaction);
void decomposeAmount(uint64_t amount, uint64_t dustThreshold, std::vector<uint64_t>& decomposedAmounts);
}
