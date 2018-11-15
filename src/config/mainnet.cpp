#include "mainnet.h"

namespace config
{
namespace mainnet
{

const char GENESIS_COINBASE_TX_HEX[] = "013c01ff000101029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd0880712101a9a4569f7e10164a32324b2b878ae32d98be0949ce6e0150ba1d7e54d60969e5";
const char P2P_STAT_TRUSTED_PUB_KEY[] = "8f80f9a5a434a9f1510d13336228debfee9c918ce505efe225d8c94d045fa115";
const uint8_t BLOCK_MAJOR_VERSION_1 = 1;
const uint8_t BLOCK_MINOR_VERSION_0 = 0;

//TODO Put here the name of your currency
const char CRYPTONOTE_NAME[] = "vigcoin";

seeds_t seeds = {
    "69.171.73.252:19800",
    "39.108.160.252:19800",
    "144.202.10.183:19800",
};

checkpoints_t checkpoints = {};
config_t data = {
    CRYPTONOTE_NAME,
    // Block Info
    {GENESIS_COINBASE_TX_HEX,
     {BLOCK_MAJOR_VERSION_1, BLOCK_MINOR_VERSION_0, 0}},
    {1, 0, 0},
    {P2P_STAT_TRUSTED_PUB_KEY,
     {1, 0, 0}},
    seeds,
    checkpoints};
}; // namespace mainnet
} // namespace config