#pragma once

#include <cstdint>
#include <cstddef>
#include <initializer_list>

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
    version_t version;
};

struct checkpoint_data_t
{
    uint32_t height;
    const char *blockId;
};

struct config_t
{
    config_block_t block;
    config_trasaction_t transaction;
    config_net_t net;
};
} // namespace config