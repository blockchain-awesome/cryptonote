#include "mainnet.h"

namespace config
{
namespace mainnet
{

namespace
{

const char GENESIS_COINBASE_TX_HEX[] = "013c01ff000101029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd0880712101a9a4569f7e10164a32324b2b878ae32d98be0949ce6e0150ba1d7e54d60969e5";
const char P2P_STAT_TRUSTED_PUB_KEY[] = "8f80f9a5a434a9f1510d13336228debfee9c918ce505efe225d8c94d045fa115";
const uint8_t BLOCK_MAJOR_VERSION_1 = 1;
const uint8_t BLOCK_MINOR_VERSION_0 = 0;
const uint8_t CURRENT_TRANSACTION_VERSION = 1;

//TODO This port will be used by the daemon to establish connections with p2p network
const uint16_t P2P_DEFAULT_PORT = 19800;
//TODO This port will be used by the daemon to interact with simlewallet
const uint16_t RPC_DEFAULT_PORT = 19801;
//TODO This port will be used by the client wallets to interact with a service node
const uint16_t RPC_WALLET_PORT = 19888;

//TODO Put here the name of your currency
const char CRYPTONOTE_NAME[] = "vigcoin";

const uint64_t CRYPTONOTE_CREATION_TIME = 1520035200000;

//Filenames
const char CRYPTONOTE_BLOCKS_FILENAME[] = "blocks.dat";
const char CRYPTONOTE_BLOCKINDEXES_FILENAME[] = "blockindexes.dat";
const char CRYPTONOTE_BLOCKSCACHE_FILENAME[] = "blockscache.dat";
const char CRYPTONOTE_POOLDATA_FILENAME[] = "poolstate.bin";
const char P2P_NET_DATA_FILENAME[] = "p2pstate.bin";
const char CRYPTONOTE_BLOCKCHAIN_INDICES_FILENAME[] = "blockchainindices.dat";
const char MINER_CONFIG_FILE_NAME[] = "miner_conf.json";

seeds_t seeds = {
    "69.171.73.252:19800",
    "39.108.160.252:19800",
    "144.202.10.183:19800",
};

checkpoints_t checkpoints = {

};

const hard_fork_list_t hardforks = {
    {1, 1, 0, 1341378000},
    {2, 185643, 0, 1546171542010}};

filename_t filenames =
    {CRYPTONOTE_BLOCKS_FILENAME,
     CRYPTONOTE_BLOCKINDEXES_FILENAME,
     CRYPTONOTE_BLOCKSCACHE_FILENAME,
     CRYPTONOTE_BLOCKCHAIN_INDICES_FILENAME,
     CRYPTONOTE_POOLDATA_FILENAME,
     P2P_NET_DATA_FILENAME,
     MINER_CONFIG_FILE_NAME};

storage_version_t storage = {
    {1, 0, 0},
    {1, 0, 0}};

} // namespace

config_t data = {
    CRYPTONOTE_NAME,
    CRYPTONOTE_CREATION_TIME,
    // Block Info
    {GENESIS_COINBASE_TX_HEX,
     {BLOCK_MAJOR_VERSION_1, BLOCK_MINOR_VERSION_0, 0}},
    //Transaction Info
    {CURRENT_TRANSACTION_VERSION, 0, 0},
    {P2P_STAT_TRUSTED_PUB_KEY,
     P2P_DEFAULT_PORT,
     RPC_DEFAULT_PORT,
     RPC_WALLET_PORT,
     {1, 0, 0}},
    seeds,
    checkpoints,
    hardforks,
    storage,
    filenames};
}; // namespace mainnet
} // namespace config