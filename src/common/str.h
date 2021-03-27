#pragma once

#include <string>

namespace string
{
  class IPv4
  {
  public:
    static std::string to(uint32_t ip);
    static uint32_t from(std::string &ip);
  };

  class Port
  {
  public:
    static uint32_t from(std::string &ip);
  };

  class Time
  {
  public:
    static std::string ago(uint64_t seconds);
  };

  class Base64
  {
    static const std::string chars;

  public:
    static bool valid(unsigned char c);
    static std::string decode(std::string const &encoded_string);
  };

}