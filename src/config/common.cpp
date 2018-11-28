#include "common.h"
#include "mainnet.h"
#include "testnet.h"
#include <iostream>

namespace config
{
config_enum_t type = MAINNET;

void setType(config_enum_t t)
{
  type = t;
}

bool isType(config_enum_t t)
{
  return type == t;
}

config_t &get()
{
  std::cout << "get config tyep : " << type << std::endl;
  switch (type)
  {
  case MAINNET:
    return mainnet::data;
  default:
    return testnet::data;
  }
}
} // namespace config