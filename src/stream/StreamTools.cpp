// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StreamTools.h"
#include <stdexcept>
#include "IOutputStream.h"

namespace Common {

void write(IOutputStream& out, const void* data, size_t size) {
  while (size > 0) {
    size_t writtenSize = out.writeSome(data, size);
    if (writtenSize == 0) {
      throw std::runtime_error("Failed to write to IOutputStream");
    }

    data = static_cast<const uint8_t*>(data) + writtenSize;
    size -= writtenSize;
  }
}

void write(IOutputStream& out, int8_t value) {
  write(out, &value, sizeof(value));
}

void write(IOutputStream& out, int16_t value) {
  // TODO: Convert to little endian on big endian platforms
  write(out, &value, sizeof(value));
}

void write(IOutputStream& out, int32_t value) {
  // TODO: Convert to little endian on big endian platforms
  write(out, &value, sizeof(value));
}

void write(IOutputStream& out, int64_t value) {
  // TODO: Convert to little endian on big endian platforms
  write(out, &value, sizeof(value));
}

void write(IOutputStream& out, uint8_t value) {
  write(out, &value, sizeof(value));
}

void write(IOutputStream& out, uint16_t value) {
  // TODO: Convert to little endian on big endian platforms
  write(out, &value, sizeof(value));
}

void write(IOutputStream& out, uint32_t value) {
  // TODO: Convert to little endian on big endian platforms
  write(out, &value, sizeof(value));
}

void write(IOutputStream& out, uint64_t value) {
  // TODO: Convert to little endian on big endian platforms
  write(out, &value, sizeof(value));
}

void write(IOutputStream& out, const std::vector<uint8_t>& data) {
  write(out, data.data(), data.size());
}

void write(IOutputStream& out, const std::string& data) {
  write(out, data.data(), data.size());
}

void writeVarint(IOutputStream& out, uint32_t value) {
  while (value >= 0x80) {
    write(out, static_cast<uint8_t>(value | 0x80));
    value >>= 7;
  }

  write(out, static_cast<uint8_t>(value));
}

void writeVarint(IOutputStream& out, uint64_t value) {
  while (value >= 0x80) {
    write(out, static_cast<uint8_t>(value | 0x80));
    value >>= 7;
  }

  write(out, static_cast<uint8_t>(value));
}

}
