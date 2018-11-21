// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StringTools.h"
#include <fstream>

namespace Common {

std::string extract(std::string& text, char delimiter) {
  size_t delimiterPosition = text.find(delimiter);
  std::string subText;
  if (delimiterPosition != std::string::npos) {
    subText = text.substr(0, delimiterPosition);
    text = text.substr(delimiterPosition + 1);
  } else {
    subText.swap(text);
  }

  return subText;
}

std::string extract(const std::string& text, char delimiter, size_t& offset) {
  size_t delimiterPosition = text.find(delimiter, offset);
  if (delimiterPosition != std::string::npos) {
    offset = delimiterPosition + 1;
    return text.substr(offset, delimiterPosition);
  } else {
    offset = text.size();
    return text.substr(offset);
  }
}

namespace {

static const std::string base64chars =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789+/";

bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

}

std::string base64Decode(std::string const& encoded_string) {
  size_t in_len = encoded_string.size();
  size_t i = 0;
  size_t j = 0;
  size_t in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i == 4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = (unsigned char)base64chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = (unsigned char)base64chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}


bool loadFileToString(const std::string& filepath, std::string& buf) {
  try {
    std::ifstream fstream;
    fstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fstream.open(filepath, std::ios_base::binary | std::ios_base::in | std::ios::ate);

    size_t fileSize = static_cast<size_t>(fstream.tellg());
    buf.resize(fileSize);

    if (fileSize > 0)  {
      fstream.seekg(0, std::ios::beg);
      fstream.read(&buf[0], buf.size());
    }
  } catch (const std::exception&) {
    return false;
  }

  return true;
}

bool saveStringToFile(const std::string& filepath, const std::string& buf) {
  try {
    std::ofstream fstream;
    fstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fstream.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    fstream << buf;
  } catch (const std::exception&)  {
    return false;
  }

  return true;
}


std::string ipAddressToString(uint32_t ip) {
  uint8_t bytes[4];
  bytes[0] = ip & 0xFF;
  bytes[1] = (ip >> 8) & 0xFF;
  bytes[2] = (ip >> 16) & 0xFF;
  bytes[3] = (ip >> 24) & 0xFF;

  char buf[16];
  sprintf(buf, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);

  return std::string(buf);
}

bool parseIpAddressAndPort(uint32_t& ip, uint32_t& port, const std::string& addr) {
  uint32_t v[4];
  uint32_t localPort;

  if (sscanf(addr.c_str(), "%d.%d.%d.%d:%d", &v[0], &v[1], &v[2], &v[3], &localPort) != 5) {
    return false;
  }

  for (int i = 0; i < 4; ++i) {
    if (v[i] > 0xff) {
      return false;
    }
  }

  ip = (v[3] << 24) | (v[2] << 16) | (v[1] << 8) | v[0];
  port = localPort;
  return true;
}

std::string timeIntervalToString(uint64_t intervalInSeconds) {
  auto tail = intervalInSeconds;

  auto days = tail / (60 * 60 * 24);
  tail = tail % (60 * 60 * 24);
  auto hours = tail / (60 * 60);
  tail = tail % (60 * 60);
  auto  minutes = tail / (60);
  tail = tail % (60);
  auto seconds = tail;

  return 
    "d" + std::to_string(days) + 
    ".h" + std::to_string(hours) + 
    ".m" + std::to_string(minutes) +
    ".s" + std::to_string(seconds);
}


}
