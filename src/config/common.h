#pragma once

#include <cstdint>
#include <cstddef>
#include <ctime>
#include <initializer_list>
#include <vector>

namespace config
{

struct version_t
{
    uint8_t major;
    uint8_t miner;
    uint8_t patch;
};

struct config_block_t
{
    const char *genesis_coinbase_tx_hex;
    version_t version;
};

struct config_trasaction_t
{
    version_t version;
};

struct config_net_t
{
    const char *p2p_stat_trusted_pub_key;
    uint16_t p2p_port;
    uint16_t rpc_port;
    uint16_t wallet_port;
    version_t version;
};

struct checkpoint_data_t
{
    uint32_t height;
    const char *blockId;
};

typedef std::initializer_list<const char *> seeds_t;

typedef std::initializer_list<checkpoint_data_t> checkpoints_t;

struct hard_fork_t {
  uint8_t version;
  uint64_t height;
  uint8_t threshold;
  time_t time;
};

typedef std::vector<hard_fork_t> hard_fork_list_t;


struct storage_t {
    version_t blockcache_archive;
    version_t blockcache_indices_archive;
};

struct config_t
{
    const char *name;
    uint64_t createTime;
    config_block_t block;
    config_trasaction_t transaction;
    config_net_t net;
    seeds_t seeds;
    checkpoints_t checkpoints;
    hard_fork_list_t hardforks;
    storage_t storage;
};

enum config_enum_t
{
    MAINNET = 1,
    TESTNET
};


extern void setType(config_enum_t t);
extern bool isType(config_enum_t t);
extern config_t &get();

} // namespace config