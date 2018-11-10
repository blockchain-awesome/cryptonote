#include "mainnet.h"

namespace config
{
namespace mainnet
{
const char GENESIS_COINBASE_TX_HEX[] = "013c01ff000101029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd0880712101a9a4569f7e10164a32324b2b878ae32d98be0949ce6e0150ba1d7e54d60969e5";

config_t data = {
    {GENESIS_COINBASE_TX_HEX,
     {1, 0, 0}},
    {1, 0, 0},
    {"8f80f9a5a434a9f1510d13336228debfee9c918ce505efe225d8c94d045fa115",
     {1, 0, 0}}};
}; // namespace mainnet
} // namespace config