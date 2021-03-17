// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace Common {

class IOutputStream;

void write(IOutputStream& out, const void* data, size_t size);
void write(IOutputStream& out, int8_t value);
void write(IOutputStream& out, int16_t value);
void write(IOutputStream& out, int32_t value);
void write(IOutputStream& out, int64_t value);
void write(IOutputStream& out, uint8_t value);
void write(IOutputStream& out, uint16_t value);
void write(IOutputStream& out, uint32_t value);
void write(IOutputStream& out, uint64_t value);
void write(IOutputStream& out, const std::vector<uint8_t>& data);
void write(IOutputStream& out, const std::string& data);
void writeVarint(IOutputStream& out, uint64_t value);

};
