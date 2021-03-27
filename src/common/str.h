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
    std::string ago(uint64_t seconds);
  };
}