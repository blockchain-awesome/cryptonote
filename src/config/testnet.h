#pragma once

#include "common.h"

namespace config
{
namespace testnet
{
extern config_t data;
extern seeds_t seeds;
extern checkpoints_t checkpoints;
namespace storage
{
extern const version_t blockcache_archive;
extern const version_t blockcache_indices_archive;
} // namespace storage

} // namespace testnet
} // namespace config