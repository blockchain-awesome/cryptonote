// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "writer.h"

namespace Common {

void write(Writer& out, const void* data, size_t size);
void write(Writer& out, int8_t value);
void write(Writer& out, int16_t value);
void write(Writer& out, int32_t value);
void write(Writer& out, int64_t value);
void write(Writer& out, uint8_t value);
void write(Writer& out, uint16_t value);
void write(Writer& out, uint32_t value);
void write(Writer& out, uint64_t value);
void write(Writer& out, const std::vector<uint8_t>& data);
void write(Writer& out, const std::string& data);
void writeVarint(Writer& out, uint64_t value);

};
