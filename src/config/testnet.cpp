#include "testnet.h"

namespace config
{
namespace testnet
{
const char GENESIS_COINBASE_TX_HEX[] = "013c01ff000101029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd0880712101398fb9ec4e76aeef124dfb779de715022efd619e63d7516f8b1470266f5da1fd";
const char P2P_STAT_TRUSTED_PUB_KEY[] = "8f80f9a5a434a9f1510d13336228debfee9c918ce505efe225d8c94d045fa115";

seeds_t seeds = {
    "69.171.73.252:19800",
    "39.108.160.252:19800",
    "144.202.10.183:19800",
};

checkpoints_t checkpoints = {};
config_t data = {
    {GENESIS_COINBASE_TX_HEX,
     {1, 0, 0}},
    {1, 0, 0},
    {P2P_STAT_TRUSTED_PUB_KEY,
     {1, 0, 0}},
    seeds,
    checkpoints};
}; // namespace testnet
} // namespace config