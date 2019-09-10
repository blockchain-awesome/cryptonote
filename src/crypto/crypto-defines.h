
#pragma once

#include <stdint.h>

typedef struct {
  uint8_t data[32];
} elliptic_curve_point_t;

typedef struct  {
  uint8_t data[32];
} elliptic_curve_scalar_t;

void random_scalar(uint8_t *res);
