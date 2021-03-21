// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "reader.h"
#include <algorithm>
#include <cassert>
#include <cstring> // memcpy

namespace Common
{

  // Reader::MemoryInputStream(const void *buffer, size_t bufferSize) : buffer(static_cast<const char *>(buffer)), bufferSize(bufferSize),
  //                                                                               position(0), mem((char *)buffer, (char *)buffer + bufferSize),
  //                                                                               temp(&mem), Reader(temp)
  // {
  // }

  // size_t Reader::getPosition() const
  // {
  //   return in.tellg();
  // }

  // bool Reader::endOfStream() const
  // {
  //   return in.peek() == EOF;
  // }

  // size_t Reader::readSome(void *data, size_t size)
  // {
  //   assert(position <= bufferSize);
  //   size_t readSize = std::min(size, bufferSize - position);

  //   if (readSize > 0)
  //   {
  //     memcpy(data, buffer + position, readSize);
  //     position += readSize;
  //   }

  //   return readSize;
  // }

}
