#include "common.h"
#include "mainnet.h"
#include "testnet.h"

namespace config
{
config_enum_t type = MAINNET;

void setType(config_enum_t t)
{
    type = t;
}

config_t &get()
{
    switch (type)
    {
    case TESTNET:
        return testnet::data;
    default:
        return mainnet::data;
    }
}
} // namespace config