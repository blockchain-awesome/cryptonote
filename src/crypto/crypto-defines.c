
#include <stdint.h>
#include <memory.h>
#include "crypto-ops.h"
#include "crypto-defines.h"
#include "random.h"

void random_scalar(uint8_t *res)
{
  unsigned char tmp[64];
  generate_random_bytes_not_thread_safe(64, tmp);
  sc_reduce(tmp);
  memcpy(res, tmp, 32);
}