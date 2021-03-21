// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cryptonote.h>
#include "BinaryInputStreamSerializer.h"
#include "BinaryOutputStreamSerializer.h"
#include "stream/reader.h"
#include "stream/reader.h"
#include "stream/writer.h"
#include "stream/VectorOutputStream.h"

#include <fstream>

namespace cryptonote {

template <typename T>
binary_array_t storeToBinary(const T& obj) {
  binary_array_t result;
  Common::VectorOutputStream stream(result);
  BinaryOutputStreamSerializer ba(stream);
  serialize(const_cast<T&>(obj), ba);
  return result;
}

template <typename T>
void loadFromBinary(T& obj, const binary_array_t& blob) {
  // Reader stream(blob.data(), blob.size());

  const unsigned char * b = static_cast<const unsigned char *>(blob.data());
  membuf mem((char *)(b), (char *)(b + blob.size()));
  std::istream istream(&mem);
  Reader stream(istream);
  BinaryInputStreamSerializer ba(stream);
  serialize(obj, ba);
}

template <typename T>
bool storeToBinaryFile(const T& obj, const std::string& filename) {
  try {
    std::ofstream dataFile;
    dataFile.open(filename, std::ios_base::binary | std::ios_base::out | std::ios::trunc);
    if (dataFile.fail()) {
      return false;
    }

    Writer stream(dataFile);
    BinaryOutputStreamSerializer out(stream);
    cryptonote::serialize(const_cast<T&>(obj), out);
      
    if (dataFile.fail()) {
      return false;
    }

    dataFile.flush();
  } catch (std::exception&) {
    return false;
  }

  return true;
}

template<class T>
bool loadFromBinaryFile(T& obj, const std::string& filename) {
  try {
    std::ifstream dataFile;
    dataFile.open(filename, std::ios_base::binary | std::ios_base::in);
    if (dataFile.fail()) {
      std::cout << "Fail to open a binary file: " << filename << "!" << std::endl;
      std::cout << "Error Reason: " << strerror(errno) << "!" << std::endl;
      return false;
    }

    Reader stream(dataFile);
    BinaryInputStreamSerializer in(stream);
    serialize(obj, in);
    return !dataFile.fail();
  } catch (std::exception& e) {
    std::cout << "Load exception :" << e.what() << std::endl;
    return false;
  }
}

}
