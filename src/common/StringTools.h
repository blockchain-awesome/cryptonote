// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "hex.h"

namespace Common {

std::string extract(std::string& text, char delimiter); // Does not throw
std::string extract(const std::string& text, char delimiter, size_t& offset); // Does not throw

template<typename T> T fromString(const std::string& text) { // Throws on error
  T value;
  std::istringstream stream(text);
  stream >> value;
  if (stream.fail()) {
    throw std::runtime_error("fromString: unable to parse value");
  }

  return value;
}

template<typename T> bool fromString(const std::string& text, T& value) { // Does not throw
  std::istringstream stream(text);
  stream >> value;
  return !stream.fail();
}

template<typename T> std::vector<T> fromDelimitedString(const std::string& source, char delimiter) { // Throws on error
  std::vector<T> data;
  for (size_t offset = 0; offset != source.size();) {
    data.emplace_back(fromString<T>(extract(source, delimiter, offset)));
  }

  return data;
}

template<typename T> bool fromDelimitedString(const std::string& source, char delimiter, std::vector<T>& data) { // Does not throw
  for (size_t offset = 0; offset != source.size();) {
    T value;
    if (!fromString<T>(extract(source, delimiter, offset), value)) {
      return false;
    }

    data.emplace_back(value);
  }

  return true;
}

template<typename T> std::string toString(const T& value) { // Does not throw
  std::ostringstream stream;
  stream << value;
  return stream.str();
}

template<typename T> void toString(const T& value, std::string& text) { // Does not throw
  std::ostringstream stream;
  stream << value;
  text += stream.str();
}

bool loadFileToString(const std::string& filepath, std::string& buf);
bool saveStringToFile(const std::string& filepath, const std::string& buf);


std::string base64Decode(std::string const& encoded_string);

std::string ipAddressToString(uint32_t ip);
bool parseIpAddressAndPort(uint32_t& ip, uint32_t& port, const std::string& addr);

std::string timeIntervalToString(uint64_t intervalInSeconds);

}
