#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
namespace coin
{
const char name[] = "vigcoin";
struct Difficulty
{
  uint64_t target;
  size_t window;
  size_t cut;
  size_t blockCount;
};

struct StorageFiles
{
  std::string blocks;
  std::string blocksCache;
  std::string blocksIndexes;
  std::string txPool;
  std::string blockchainIndexes;
};

struct FusionTx
{
  size_t maxSize;
  size_t minInputCount;
  size_t minInOutCountRatio;
};

struct maxBlockSize
{
  size_t initial;
  uint64_t growthSpeedNumerator;
  uint64_t growthSpeedDenominator;
};

} // namespace coin