
#include "amount.h"
#include "assert.h"
#include <limits>
#include "crypto/util.h"

uint64_t getPenalizedAmount(uint64_t amount, size_t medianSize, size_t currentBlockSize)
{
  static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t is too small");
  assert(currentBlockSize <= 2 * medianSize);
  assert(medianSize <= std::numeric_limits<uint32_t>::max());
  assert(currentBlockSize <= std::numeric_limits<uint32_t>::max());

  if (amount == 0)
  {
    return 0;
  }

  if (currentBlockSize <= medianSize)
  {
    return amount;
  }

  uint64_t productHi;
  uint64_t productLo = mul128(amount, currentBlockSize * (UINT64_C(2) * medianSize - currentBlockSize), &productHi);

  uint64_t penalizedAmountHi;
  uint64_t penalizedAmountLo;
  div128_32(productHi, productLo, static_cast<uint32_t>(medianSize), &penalizedAmountHi, &penalizedAmountLo);
  div128_32(penalizedAmountHi, penalizedAmountLo, static_cast<uint32_t>(medianSize), &penalizedAmountHi, &penalizedAmountLo);

  assert(0 == penalizedAmountHi);
  assert(penalizedAmountLo < amount);

  return penalizedAmountLo;
}