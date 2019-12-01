
#include "amount.h"
#include "assert.h"
#include <limits>
#include "crypto/util.h"

uint64_t get_penalized_amount(uint64_t amount, size_t median_size, size_t current_block_size)
{
  static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t is too small");
  assert(current_block_size <= 2 * median_size);
  assert(median_size <= std::numeric_limits<uint32_t>::max());
  assert(current_block_size <= std::numeric_limits<uint32_t>::max());

  if (amount == 0)
  {
    return 0;
  }

  if (current_block_size <= median_size)
  {
    return amount;
  }

  uint64_t product_high;
  uint64_t product_low = mul128(amount, current_block_size * (UINT64_C(2) * median_size - current_block_size), &product_high);

  uint64_t penalized_amount_high;
  uint64_t penalized_amount_low;
  div128_32(product_high, product_low, static_cast<uint32_t>(median_size), &penalized_amount_high, &penalized_amount_low);
  div128_32(penalized_amount_high, penalized_amount_low, static_cast<uint32_t>(median_size), &penalized_amount_high, &penalized_amount_low);

  assert(0 == penalized_amount_high);
  assert(penalized_amount_low < amount);

  return penalized_amount_low;
}