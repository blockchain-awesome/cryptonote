#include <string>
#include <cstring>

#include "str.h"

namespace string
{

  std::string IPv4::to(uint32_t ip)
  {
    uint8_t bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;

    char buf[16];
    sprintf(buf, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);

    return std::string(buf);
  }

  uint32_t IPv4::from(std::string &ip)
  {
    uint32_t v[4];
    if (sscanf(ip.c_str(), "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]) != 4)
    {
      return UINT32_MAX;
    }

    for (int i = 0; i < 4; ++i)
    {
      if (v[i] > 0xff)
      {
        return UINT32_MAX;
      }
    }

    return (v[3] << 24) | (v[2] << 16) | (v[1] << 8) | v[0];
  }

  uint32_t Port::from(std::string &str)
  {
    uint32_t port;
    if (sscanf(str.c_str(), "%d", &port) != 1)
    {
      return -1;
    }
    return port;
  }

  std::string Time::ago(uint64_t seconds)
  {

    uint64_t DAY_SECONDS = (60 * 60 * 24);
    uint64_t HOUR_SECONDS = (60 * 60);
    uint64_t MINUTE_SECONDS = (60);

    uint64_t remained = seconds;

    uint64_t days = remained / DAY_SECONDS;

    remained = remained % DAY_SECONDS;

    uint64_t hours = remained / HOUR_SECONDS;
    remained = remained % HOUR_SECONDS;

    uint64_t minutes = remained / MINUTE_SECONDS;
    remained = remained % MINUTE_SECONDS;

    char buf[255];
    memset(buf, 0, 255);
    sprintf(buf, "%lu day(s) %lu hour(s) %lu minute(s) %lu second(s)", days, hours, minutes, remained);

    return std::string(buf);
  }

  const std::string Base64::chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/";

  bool Base64::valid(unsigned char c)
  {
    return (isalnum(c) || (c == '+') || (c == '/'));
  }

  std::string Base64::decode(std::string const &encoded_string)
  {
    size_t in_len = encoded_string.size();
    size_t i = 0;
    size_t j = 0;
    size_t in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && valid(encoded_string[in_]))
    {
      char_array_4[i++] = encoded_string[in_];
      in_++;
      if (i == 4)
      {
        for (i = 0; i < 4; i++)
          char_array_4[i] = (unsigned char)chars.find(char_array_4[i]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (i = 0; (i < 3); i++)
          ret += char_array_3[i];
        i = 0;
      }
    }

    if (i)
    {
      for (j = i; j < 4; j++)
        char_array_4[j] = 0;

      for (j = 0; j < 4; j++)
        char_array_4[j] = (unsigned char)chars.find(char_array_4[j]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (j = 0; (j < i - 1); j++)
        ret += char_array_3[j];
    }

    return ret;
  }
}
