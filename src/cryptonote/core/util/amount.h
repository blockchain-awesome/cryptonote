#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C"
{
  uint64_t getPenalizedAmount(uint64_t amount, size_t medianSize, size_t currentBlockSize);
}