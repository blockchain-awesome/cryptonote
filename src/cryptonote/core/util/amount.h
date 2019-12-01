#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C"
{
  uint64_t get_penalized_amount(uint64_t amount, size_t median_size, size_t current_block_size);
}