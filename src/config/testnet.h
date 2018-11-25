#pragma once

#include "common.h"
#include <vector>

namespace config
{
namespace testnet
{
extern config_t data;
extern seeds_t seeds;
extern checkpoints_t checkpoints;
extern const std::vector<hard_fork_t> hardforks;

namespace storage
{
extern const version_t blockcache_archive;
extern const version_t blockcache_indices_archive;
} // namespace storage

} // namespace testnet
} // namespace config