// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "hex.h"

namespace Common {
std::string base64Decode(std::string const& encoded_string);

// std::string ipAddressToString(uint32_t ip);
// bool parseIpAddressAndPort(uint32_t& ip, uint32_t& port, const std::string& addr);

std::string timeIntervalToString(uint64_t intervalInSeconds);

}
